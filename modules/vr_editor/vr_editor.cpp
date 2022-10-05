/*************************************************************************/
/*  vr_editor.cpp                                                        */
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

#include "vr_editor.h"

#include "editor/filesystem_dock.h"
#include "editor/import_dock.h"
#include "editor/inspector_dock.h"
#include "editor/node_dock.h"
#include "editor/scene_tree_dock.h"
#include "servers/xr/xr_interface.h"
#include "servers/xr_server.h"

bool VREditor::init_editor(EditorNode *p_editor_node, bool p_is_standalone) {
	Ref<XRInterface> xr_interface = XRServer::get_singleton()->find_interface("OpenXR");
	if (xr_interface.is_valid() && xr_interface->is_initialized()) {
		Viewport *vp;
		if (p_is_standalone) {
			// We use our main viewport for HMD output, this means our desktop editor should NOT be setup.
			vp = p_editor_node->get_tree()->get_root();
		} else {
			// We use a separate viewport for HMD output, this can exist side by side with our desktop editor.
			SubViewport *svp = memnew(SubViewport);
			svp->set_clear_mode(SubViewport::CLEAR_MODE_ALWAYS);
			svp->set_update_mode(SubViewport::UPDATE_ALWAYS); // Our OpenXR interface should disable rendering the viewport if the player isn't wearing the headset
			svp->set_as_audio_listener_3d(true);

			// should further configure this viewport with other needed settings such as shadow map support

			p_editor_node->add_child(svp);
			vp = svp;
		}

		// Make sure V-Sync is OFF or our monitor frequency will limit our headset
		// TODO improve this to only override v-sync when the player is wearing the headset
		DisplayServer::get_singleton()->window_set_vsync_mode(DisplayServer::VSYNC_DISABLED);

		// Enable our viewport for VR use
		vp->set_vrs_mode(Viewport::VRS_XR);
		vp->set_use_xr(true);

		// Now add our VR editor
		VREditor *vr_pm = memnew(VREditor(p_editor_node, vp, p_is_standalone));
		vp->add_child(vr_pm);

		return true;
	} else {
		// Our XR interface was never setup properly, for now assume this means we're not running in XR mode
		// TODO improve this so if we were meant to be in XR mode but failed, especially if we're stand alone, we should hard exit.
		return false;
	}
}

VREditor::VREditor(EditorNode *p_editor_node, Viewport *p_xr_viewport, bool p_is_standalone) {
	is_standalone = p_is_standalone;
	editor_node = p_editor_node;
	xr_viewport = p_xr_viewport;

	// Add our avatar
	avatar = memnew(VREditorAvatar);
	xr_viewport->add_child(avatar);

	// Add our basic UI
	// TODO make sizes settable/saveable
	// TODO deside if we want a separate VR scale for EDSCALE

	theme = create_custom_theme();

	// left side docks
	left_dock = memnew(VRWindow(Size2i(500, 1000) * EDSCALE));
	left_dock->set_position(Vector3(-0.5, 0.0, 0.0));
	left_dock->set_rotation(Vector3(0.0, 10.0 * Math_PI / 180.0, 0.0));
	avatar->add_window_to_hud(left_dock);

	Control *left_theme_base = memnew(Control);
	left_dock->get_scene_root()->add_child(left_theme_base);
	left_theme_base->set_theme(theme);
	left_theme_base->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

	HSplitContainer *left_hsplit = memnew(HSplitContainer);
	left_theme_base->add_child(left_hsplit);
	left_hsplit->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	left_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	VSplitContainer *left_l_vsplit = memnew(VSplitContainer);
	left_hsplit->add_child(left_l_vsplit);
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UL] = memnew(TabContainer);
	left_l_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_LEFT_UL]);
	dock_slot[EditorNode::DOCK_SLOT_LEFT_BL] = memnew(TabContainer);
	left_l_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_LEFT_BL]);

	VSplitContainer *left_r_vsplit = memnew(VSplitContainer);
	left_hsplit->add_child(left_r_vsplit);
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UR] = memnew(TabContainer);
	left_r_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]);
	dock_slot[EditorNode::DOCK_SLOT_LEFT_BR] = memnew(TabContainer);
	left_r_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_LEFT_BR]);

	// right side docks
	right_dock = memnew(VRWindow(Size2i(500, 1000) * EDSCALE));
	right_dock->set_position(Vector3(0.5, 0.0, 0.0));
	right_dock->set_rotation(Vector3(0.0, -10.0 * Math_PI / 180.0, 0.0));
	avatar->add_window_to_hud(right_dock);

	Control *right_theme_base = memnew(Control);
	right_dock->get_scene_root()->add_child(right_theme_base);
	right_theme_base->set_theme(theme);
	right_theme_base->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

	HSplitContainer *right_hsplit = memnew(HSplitContainer);
	right_theme_base->add_child(right_hsplit);
	right_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	VSplitContainer *right_l_vsplit = memnew(VSplitContainer);
	right_hsplit->add_child(right_l_vsplit);
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL] = memnew(TabContainer);
	right_l_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]);
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_BL] = memnew(TabContainer);
	right_l_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_RIGHT_BL]);

	VSplitContainer *right_r_vsplit = memnew(VSplitContainer);
	right_hsplit->add_child(right_r_vsplit);
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UR] = memnew(TabContainer);
	right_r_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_RIGHT_UR]);
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_BR] = memnew(TabContainer);
	right_r_vsplit->add_child(dock_slot[EditorNode::DOCK_SLOT_RIGHT_BR]);

	for (int i = 0; i < EditorNode::DOCK_SLOT_MAX; i++) {
		dock_slot[i]->set_custom_minimum_size(Size2(170, 0) * EDSCALE);
		dock_slot[i]->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		// dock_slot[i]->set_popup(dock_select_popup);
		// dock_slot[i]->connect("pre_popup_pressed", callable_mp(this, &EditorNode::_dock_pre_popup).bind(i));
		dock_slot[i]->set_drag_to_rearrange_enabled(true);
		dock_slot[i]->set_tabs_rearrange_group(1);
		// dock_slot[i]->connect("tab_changed", callable_mp(this, &EditorNode::_dock_tab_changed));
		dock_slot[i]->set_use_hidden_tabs_for_min_size(true);
	}

	/* Add our build in docks in their default locations, we'll have a lot of work ahead of us recreating the logic in the main editor */

	// For now we're sharing these with our main editor
	SubViewport *scene_root = editor_node->get_scene_root();
	EditorData &editor_data = EditorNode::get_editor_data();
	EditorSelection *editor_selection = editor_node->get_editor_selection();

	// We're creating additional instances here for objects originally designed as singletons.
	// TODO much more work needs to be done to rework these UI components to allow for multiple instances to exist.
	SceneTreeDock *scene_tree_dock = memnew(SceneTreeDock(scene_root, editor_selection, editor_data));
	// memnew(InspectorDock(editor_data));
	// memnew(ImportDock);
	// memnew(NodeDock);

	// FileSystemDock *filesystem_dock = memnew(FileSystemDock);
	// TODO implement these
	// filesystem_dock->connect("inherit", callable_mp(this, &EditorNode::_inherit_request));
	// filesystem_dock->connect("instance", callable_mp(this, &EditorNode::_instantiate_request));
	// filesystem_dock->connect("display_mode_changed", callable_mp(this, &EditorNode::_save_docks));
	// get_project_settings()->connect_filesystem_dock_signals(filesystem_dock);

	// Scene: Top left.
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->add_child(scene_tree_dock);
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->set_tab_title(dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->get_tab_idx_from_control(scene_tree_dock), TTR("Scene"));

	/*

	// Import: Top left, behind Scene.
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->add_child(ImportDock::get_singleton());
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->set_tab_title(dock_slot[EditorNode::DOCK_SLOT_LEFT_UR]->get_tab_idx_from_control(ImportDock::get_singleton()), TTR("Import"));

	// FileSystem: Bottom left.
	dock_slot[EditorNode::DOCK_SLOT_LEFT_BR]->add_child(FileSystemDock::get_singleton());
	dock_slot[EditorNode::DOCK_SLOT_LEFT_BR]->set_tab_title(dock_slot[EditorNode::DOCK_SLOT_LEFT_BR]->get_tab_idx_from_control(FileSystemDock::get_singleton()), TTR("FileSystem"));

	// Inspector: Full height right.
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->add_child(InspectorDock::get_singleton());
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->set_tab_title(dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->get_tab_idx_from_control(InspectorDock::get_singleton()), TTR("Inspector"));

	// Node: Full height right, behind Inspector.
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->add_child(NodeDock::get_singleton());
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->set_tab_title(dock_slot[EditorNode::DOCK_SLOT_RIGHT_UL]->get_tab_idx_from_control(NodeDock::get_singleton()), TTR("Node"));

	*/

	// Hide unused dock slots and vsplits.
	dock_slot[EditorNode::DOCK_SLOT_LEFT_UL]->hide();
	dock_slot[EditorNode::DOCK_SLOT_LEFT_BL]->hide();
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_BL]->hide();
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_UR]->hide();
	dock_slot[EditorNode::DOCK_SLOT_RIGHT_BR]->hide();
	left_l_vsplit->hide();
	right_r_vsplit->hide();
}

VREditor::~VREditor() {
}
