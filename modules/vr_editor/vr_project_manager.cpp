/*************************************************************************/
/*  vr_project_manager.cpp                                               */
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

#include "vr_project_manager.h"

#include "editor/progress_dialog.h"
#include "editor/project_manager.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/primitive_meshes.h"
#include "servers/xr/xr_interface.h"
#include "servers/xr_server.h"

void VRProjectManager::input(const Ref<InputEvent> &p_event) {
	if (!is_standalone) {
		ERR_FAIL_NULL(xr_viewport);

		// Only forward key based events...
		if (p_event->is_class("InputEventKey")) {
			xr_viewport->push_input(p_event, false);
		} else if (p_event->is_class("InputEventAction")) {
			xr_viewport->push_input(p_event, false);
		}
	}
}

bool VRProjectManager::init_project_manager(SceneTree *p_scene_tree, bool p_is_standalone) {
	Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	if (xr_interface.is_valid() && xr_interface->is_initialized()) {
		Viewport *vp;
		if (p_is_standalone) {
			// We use our main viewport for HMD output, this means our desktop project manager should NOT be setup.
			vp = p_scene_tree->get_root();
		} else {
			// We use a separate viewport for HMD output, this can exist side by side with our desktop project manager.
			SubViewport *svp = memnew(SubViewport);
			svp->set_clear_mode(SubViewport::CLEAR_MODE_ALWAYS);
			svp->set_update_mode(SubViewport::UPDATE_ALWAYS); // Our OpenXR interface should disable rendering the viewport if the player isn't wearing the headset
			svp->set_as_audio_listener_3d(true);

			// should further configure this viewport with other needed settings such as shadow map support

			p_scene_tree->get_root()->add_child(svp);
			vp = svp;
		}

		// Make sure V-Sync is OFF or our monitor frequency will limit our headset
		// TODO improve this to only override v-sync when the player is wearing the headset
		DisplayServer::get_singleton()->window_set_vsync_mode(DisplayServer::VSYNC_DISABLED);

		// Enable our viewport for VR use
		vp->set_vrs_mode(Viewport::VRS_XR);
		vp->set_use_xr(true);

		// Now add our VR project manager
		VRProjectManager *vr_pm = memnew(VRProjectManager(vp, p_is_standalone));
		vp->add_child(vr_pm);

		return true;
	} else {
		// Our XR interface was never setup properly, for now assume this means we're not running in XR mode
		// TODO improve this so if we were meant to be in XR mode but failed, especially if we're stand alone, we should hard exit.
		return false;
	}
}

VRProjectManager::VRProjectManager(Viewport *p_xr_viewport, bool p_is_standalone) {
	is_standalone = p_is_standalone;
	xr_viewport = p_xr_viewport;

	// Add our avatar
	avatar = memnew(VREditorAvatar);
	xr_viewport->add_child(avatar);

	// Add a floor so our player feels more grounded instead of floating in empty space.
	// TODO make a nice generated texture for the floor
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(Color(0.5, 0.5, 0.5, 1.0));

	Ref<PlaneMesh> mesh;
	mesh.instantiate();
	mesh->set_size(Size2(20.0, 20.0));
	mesh->set_material(material);

	MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_name("Floor");
	mesh_instance->set_mesh(mesh);
	add_child(mesh_instance);

	// For now we will render our normal project manager into a viewport and display that as a window in VR.
	// maybe some day we'll do a full 3D project manager but that is probably a waist of time to do:)

	pm_window = memnew(VRWindow(Size2i(1152, 648), 0.002));
	pm_window->set_name("ProjectManager");
	ProjectManager *pmanager = memnew(ProjectManager);
	ProgressDialog *progress_dialog = memnew(ProgressDialog);
	pmanager->add_child(progress_dialog);
	pm_window->get_scene_root()->add_child(pmanager);

	pm_window->set_position(Vector3(0.0, 0.0, -1.0));
	avatar->add_window_to_hud(pm_window);
}

VRProjectManager::~VRProjectManager() {
}
