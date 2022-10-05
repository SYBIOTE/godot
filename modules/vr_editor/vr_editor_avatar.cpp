/*************************************************************************/
/*  vr_editor_avatar.cpp                                                 */
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

#include "vr_editor_avatar.h"

#include "scene/resources/primitive_meshes.h"

/* VRPoke */

void VRPoke::set_ray_enabled(bool p_enabled) {
	ray_enabled = p_enabled;

	cast->set_visible(ray_enabled);
}

bool VRPoke::is_select() {
	// Returns true if our select action is triggered on the controller we're a child off.

	XRController3D *controller = Object::cast_to<XRController3D>(get_parent());
	ERR_FAIL_NULL_V(controller, false);

	return controller->is_button_pressed("select");
}

VRPoke::VRPoke() {
	material.instantiate();
	material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	material->set_albedo(color);

	Ref<SphereMesh> sphere_mesh;
	sphere_mesh.instantiate();
	sphere_mesh->set_radius(radius);
	sphere_mesh->set_height(radius * 2.0);
	sphere_mesh->set_radial_segments(16);
	sphere_mesh->set_rings(8);
	sphere_mesh->set_material(material);

	sphere = memnew(MeshInstance3D);
	sphere->set_mesh(sphere_mesh);
	add_child(sphere);

	Ref<BoxMesh> box_mesh;
	box_mesh.instantiate();
	box_mesh->set_size(Vector3(0.001, 0.001, 5.0));
	box_mesh->set_material(material);

	cast = memnew(MeshInstance3D);
	cast->set_mesh(box_mesh);
	cast->set_position(Vector3(0.0, 0.0, -2.5));
	add_child(cast);
}

VRPoke::~VRPoke() {
}

/* VRHand */

VRHand::VRHand(Hands p_hand) {
	// We need to make something nicer for our hands
	// maybe use hand tracking if available and see if we can support all fingers.
	// But for now just fingers on our default position is fine.

	set_name(p_hand == HAND_LEFT ? "left_hand" : "right_hand");
	set_tracker(p_hand == HAND_LEFT ? "left_hand" : "right_hand");
	set_pose_name(SNAME("tool_pose"));

	poke = memnew(VRPoke);
	poke->set_position(Vector3(0.0, 0.0, -0.01));
	add_child(poke);
}

VRHand::~VRHand() {
}

/* VREditorAvatar */

void VREditorAvatar::set_ray_active_on_hand(VRHand::Hands p_hand) {
	ray_active_on_hand = p_hand;
	left_hand->set_ray_enabled(p_hand == VRHand::HAND_LEFT);
	right_hand->set_ray_enabled(p_hand == VRHand::HAND_RIGHT);
}

void VREditorAvatar::_on_button_pressed_on_hand(const String p_action, int p_hand) {
	if (p_action == "select") {
		set_ray_active_on_hand(VRHand::Hands(p_hand));
	}
}

void VREditorAvatar::set_hud_offset(real_t p_offset) {
	hud_offset = p_offset;

	Vector3 position = hud_pivot->get_position();
	position.y = hud_offset;
	hud_pivot->set_position(position);
}

void VREditorAvatar::set_hud_distance(real_t p_distance) {
	hud_distance = p_distance;

	Vector3 position = hud_root->get_position();
	position.z = -hud_distance;
	hud_root->set_position(position);
}

void VREditorAvatar::add_window_to_hud(VRWindow *p_window) {
	ERR_FAIL_COND(windows.has(p_window));

	windows.push_back(p_window);
	hud_root->add_child(p_window);
}

void VREditorAvatar::remove_window_from_hud(VRWindow *p_window) {
	ERR_FAIL_COND(!windows.has(p_window));

	windows.erase(p_window);
	hud_root->remove_child(p_window);
}

VREditorAvatar::VREditorAvatar() {
	// TODO once https://github.com/godotengine/godot/pull/63607 is merged we need to add an enhancement
	// to make this node the "current" XROrigin3D node.
	// For now this will be the current node but if a VR project is loaded things could go haywire.

	camera = memnew(XRCamera3D);
	camera->set_name("camera");
	add_child(camera);

	// Our hud pivot will follow our camera around at a constant height.
	// TODO add a button press or other mechanism to rotate our hud pivot
	// so our hud is recentered infront of our player.
	hud_pivot = memnew(Node3D);
	hud_pivot->set_name("hud_pivot");
	hud_pivot->set_position(Vector3(0.0, 1.6 + hud_offset, 0.0)); // we don't know our eye height yet
	add_child(hud_pivot);

	// Our hud root extends our hud outwards to a certain distance away
	// from our player.
	hud_root = memnew(Node3D);
	hud_root->set_name("hud_root");
	hud_root->set_position(Vector3(0.0, 0.0, -hud_distance));
	hud_pivot->add_child(hud_root);

	// Add our hands
	left_hand = memnew(VRHand(VRHand::HAND_LEFT));
	left_hand->connect("button_pressed", callable_mp(this, &VREditorAvatar::_on_button_pressed_on_hand).bind(int(VRHand::HAND_LEFT)));
	add_child(left_hand);
	pokes.push_back(left_hand->get_poke());

	right_hand = memnew(VRHand(VRHand::HAND_RIGHT));
	right_hand->connect("button_pressed", callable_mp(this, &VREditorAvatar::_on_button_pressed_on_hand).bind(int(VRHand::HAND_RIGHT)));
	add_child(right_hand);
	pokes.push_back(right_hand->get_poke());

	// TODO add callback for select so we can activate ray on last used hand

	set_ray_active_on_hand(ray_active_on_hand);

	// Add virtual keyboard
	keyboard = memnew(VRKeyboard);
	keyboard->set_name("VRKeyboard");
	keyboard->set_rotation(Vector3(20.0 * Math_PI / 180.0, 0.0, 0.0)); // put at a slight angle for comfort
	keyboard->set_position(Vector3(0.0, -0.4, 0.2)); // should make this a setting or something we can change
	add_window_to_hud(keyboard);

	// Performance info
	for (int i = 0; i < DELTA_HISTORY; i++) {
		delta_history[i] = 0.0;
	}

	fps_label = memnew(Label3D);
	fps_label->set_name("fps");
	fps_label->set_text("FPS: 00");
	fps_label->set_font_size(16);
	fps_label->set_rotation(Vector3(10.0 * Math_PI / 180.0, -10.0 * Math_PI / 180.0, 0.0));
	fps_label->set_position(Vector3(0.5, 0.5, -0.01));
	hud_root->add_child(fps_label);

	set_process(true);

	// Our default transform logic in XROrigin3D is disabled in editor mode,
	// this should be our only active XROrigin3D node in our VR editor
	set_notify_local_transform(true);
	set_notify_transform(true);
}

VREditorAvatar::~VREditorAvatar() {
}

void VREditorAvatar::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			double delta = get_process_delta_time();

			XRPose::TrackingConfidence confidence = camera->get_tracking_confidence();
			if (confidence == XRPose::XR_TRACKING_CONFIDENCE_NONE) {
				// We are not tracking so keep things where they are, user is likely not wearing the headset
				camera_is_tracking = false;
			} else {
				// Center our hud on our camera, start by calculating our desired location
				Transform3D desired_transform = camera->get_transform();
				desired_transform.origin.y = MAX(0.5, desired_transform.origin.y + hud_offset);
				Vector3 desired_forward = desired_transform.basis.rows[2];
				desired_forward.y = 0.0;
				desired_forward.normalize();

				if (!camera_is_tracking) {
					// If we weren't tracking, reposition our HUD right away, user likely just put on their headset
					camera_is_tracking = true;

					Vector3 desired_y = Vector3(0.0, 1.0, 0.0);
					Vector3 desired_x = desired_y.cross(desired_forward).normalized();
					desired_transform.basis = Basis(desired_x, desired_y, desired_forward);

					hud_pivot->set_transform(desired_transform);
				} else {
					// If we were tracking, have HUD follow head movement, this prevents motion sickness

					// Now update our transform.
					bool update_transform = false;
					Transform3D hud_transform = hud_pivot->get_transform();

					// If our desired location is more then 10cm away,
					// we start moving until our hud is within 1 cm of
					// the desired location.
					if ((desired_transform.origin - hud_transform.origin).length() > (hud_moving ? 0.01 : 0.2)) {
						hud_transform.origin = hud_transform.origin.lerp(desired_transform.origin, delta);
						hud_moving = true;
						update_transform = true;
					} else {
						hud_moving = false;
					}

					// Rotate our hud to follow the forward direction of our camera,
					// if we're looking to far away we start rotating until we're near dead center
					Vector3 hud_forward = hud_transform.basis.rows[2].normalized();
					real_t dot = desired_forward.dot(hud_forward);
					if (dot < (hud_centering ? 0.99 : 0.0)) {
						Vector3 cross = desired_forward.cross(hud_forward);
						hud_transform.basis.rotate(Vector3(0.0, cross.y > 0.0 ? 1.0 : -1.0, 0.0), delta * acos(dot));
						hud_centering = true;
						update_transform = true;
					} else {
						hud_centering = false;
					}

					if (update_transform) {
						hud_pivot->set_transform(hud_transform.orthonormalized());
					}
				}
			}

			// Loop trough our pokes and windows to simulate mouse events.
			// We don't use physics here because:
			// 1) it's overkill
			// 2) could interfear with physics in the scene being edited
			// 3) we're working on too small a scale
			// TODO: Change this so we find the closest window we're poking first.
			for (int i = 0; i < pokes.size(); i++) {
				bool handled = false;

				for (int w = 0; !handled && w < windows.size(); w++) {
					Transform3D transform = pokes[i]->get_global_transform();
					bool ray_enabled = pokes[i]->get_ray_enabled();
					bool is_select = pokes[i]->is_select();
					handled = windows[w]->ray_and_poke(i, transform.origin, ray_enabled ? -transform.basis.get_column(2) : Vector3(), is_select);
				}
			}

			// Performance info
			if (show_performance_info) {
				// We use delta here, we're not interested in how fast our one viewport renders,
				// but our total frame time
				delta_history[delta_history_index] = delta;
				delta_history_index = (delta_history_index + 1) % DELTA_HISTORY;

				double avg_delta = 0.0;
				for (int i = 0; i < DELTA_HISTORY; i++) {
					avg_delta += delta_history[i];
				}
				avg_delta /= DELTA_HISTORY;
				avg_delta = MAX(0.01, avg_delta);

				const double fps = 1.0 / avg_delta;
				fps_label->set_text(vformat(TTR("FPS: %d"), fps));
				fps_label->set_visible(true);
			} else {
				fps_label->set_visible(false);
			}
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
		case NOTIFICATION_TRANSFORM_CHANGED: {
			XRServer::get_singleton()->set_world_origin(get_global_transform());
		} break;
	}
}
