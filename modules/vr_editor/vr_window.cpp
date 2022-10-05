/*************************************************************************/
/*  vr_window.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "vr_window.h"

#include "core/input/input_event.h"
#include "scene/resources/material.h"
#include "scene/resources/primitive_meshes.h"

void VRWindow::_init(Viewport *p_viewport) {
	viewport = p_viewport;

	// We don't support opening new windows in VR (yet) so make sure we embed popups etc.
	viewport->set_embedding_subwindows(true);

	// Create a material so we can show our viewport.
	material.instantiate();
	material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, viewport->get_texture());

	// Create a mesh to display our viewport in.
	Ref<QuadMesh> mesh;
	mesh.instantiate();
	viewport_size = viewport->get_visible_rect().size;
	mesh->set_size(viewport_size * viewport_scale);
	mesh->set_material(material);

	// Create a mesh instance to make the mesh visible
	mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_mesh(mesh);
	add_child(mesh_instance);
}

void VRWindow::input(const Ref<InputEvent> &p_event) {
	// Only forward key based events...
	if (p_event->is_class("InputEventKey")) {
		viewport->push_input(p_event, false);
	} else if (p_event->is_class("InputEventAction")) {
		viewport->push_input(p_event, false);
	}
}

bool VRWindow::ray_and_poke(int p_device, Vector3 p_global_position, Vector3 p_global_direction, bool p_select) {
	Transform3D transform = mesh_instance->get_global_transform();
	Vector2 pos;
	bool ray_is_colliding = false;
	bool is_pressed = false;

	// Check our raycast
	if (p_global_direction != Vector3()) {
		// Get the plane of our screen
		Plane plane(transform.basis.get_column(2), transform.origin);

		// Determine intersect location
		Vector3 point;
		if (plane.intersects_ray(p_global_position, p_global_direction, &point)) {
			// xform to local space
			Vector3 local_position = transform.xform_inv(point);

			// check if we're within our confines
			Vector2 ray_pos = Vector2((0.5 * viewport_size.x) + (local_position.x / viewport_scale), (0.5 * viewport_size.y) - (local_position.y / viewport_scale));
			if (ray_pos.x >= 0.0 && ray_pos.x < viewport_size.x && ray_pos.y >= 0.0 && ray_pos.y < viewport_size.y) {
				// we are colliding
				pos = ray_pos;
				ray_is_colliding = true;
				is_pressed = p_select;
			}
		}
	}

	// Now check our poke
	Vector3 local_position = transform.xform_inv(p_global_position);
	float height = abs(local_position.z); // we don't care if we're touching the screen from behind, could just as well be that the user poked through the screen.
	if (height < press_distance) {
		// ok, we're close enough to the plane of our screen, now are we near enough?
		Vector2 poke_pos = Vector2((0.5 * viewport_size.x) + (local_position.x / viewport_scale), (0.5 * viewport_size.y) - (local_position.y / viewport_scale));
		if (poke_pos.x >= 0.0 && poke_pos.x < viewport_size.x && poke_pos.y >= 0.0 && poke_pos.y < viewport_size.y) {
			pos = poke_pos; // poke takes presidence
			is_pressed = true;
		}
	}

	if (is_pressed || ray_is_colliding) {
		// Emulate a single button mouse.
		// Whichever finger pressed the screen first wins.
		// If multiple fingers hover we probably get a bunch of movement events but users will learn quickly not to do that (or they'll have fun with it).

		if ((is_pressed && last_poke_device == p_device) || (!is_pressed && last_poke_device == -1)) {
			// Send motion event if we've moved
			if (pos != last_poke_position) {
				Ref<InputEventMouseMotion> mouse_event;
				mouse_event.instantiate();

				mouse_event->set_position(pos);
				mouse_event->set_global_position(pos);
				mouse_event->set_relative(pos - last_poke_position);
				mouse_event->set_button_mask(is_pressed ? MouseButton::LEFT : MouseButton::NONE);
				mouse_event->set_pressure(CLAMP((press_distance - height) / press_distance, 0.0, 1.0));

				last_poke_position = pos;
			}
		} else if (is_pressed && last_poke_device == -1) {
			last_poke_device = p_device;
			last_poke_position = pos;

			Ref<InputEventMouseButton> mouse_event;
			mouse_event.instantiate();

			mouse_event->set_button_index(MouseButton::LEFT);
			mouse_event->set_pressed(true);
			mouse_event->set_position(pos);
			mouse_event->set_global_position(pos);
			mouse_event->set_button_mask(MouseButton::LEFT);

			viewport->push_input(mouse_event, true);
		} else if (!is_pressed && last_poke_device == p_device) {
			last_poke_device = -1;
			last_poke_position = pos;

			Ref<InputEventMouseButton> mouse_event;
			mouse_event.instantiate();

			mouse_event->set_button_index(MouseButton::LEFT);
			mouse_event->set_pressed(false);
			mouse_event->set_position(pos);
			mouse_event->set_global_position(pos);
			mouse_event->set_button_mask(MouseButton::NONE);

			viewport->push_input(mouse_event, true);
		}

		// We're treating this as handled even if the player hasn't moved their controller,
		// we've determined we're close enough to our UI
		return true;
	}

	if (last_poke_device == p_device) {
		// this device is no longer touching our screen

		Ref<InputEventMouseButton> mouse_event;
		mouse_event.instantiate();

		mouse_event->set_button_index(MouseButton::LEFT);
		mouse_event->set_pressed(false);
		mouse_event->set_position(last_poke_position);
		mouse_event->set_global_position(last_poke_position);
		mouse_event->set_button_mask(MouseButton::NONE);

		viewport->push_input(mouse_event, true);

		last_poke_device = -1;
	}

	return false;
}

VRWindow::VRWindow(Size2i p_viewport_size, real_t p_viewport_scale) {
	viewport_scale = p_viewport_scale;

	SubViewport *subviewport = memnew(SubViewport);

	subviewport->set_size(p_viewport_size);
	subviewport->set_clear_mode(SubViewport::CLEAR_MODE_ALWAYS);
	subviewport->set_update_mode(SubViewport::UPDATE_ALWAYS); // TODO make this smarter so we only update content when changed...
	subviewport->set_disable_3d(true);

	add_child(subviewport);

	_init(subviewport);
}

VRWindow::VRWindow(Viewport *p_viewport, real_t p_viewport_scale) {
	viewport_scale = p_viewport_scale;

	_init(p_viewport);
}

VRWindow::~VRWindow() {
}
