/*************************************************************************/
/*  vr_editor.h                                                          */
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

#ifndef VR_EDITOR_H
#define VR_EDITOR_H

#include "editor/editor_node.h"
#include "editor/editor_scale.h"
#include "editor/editor_themes.h"
#include "scene/main/viewport.h"
#include "vr_editor_avatar.h"
#include "vr_window.h"

class VREditor : public Node3D {
	GDCLASS(VREditor, Node3D);

private:
	bool is_standalone = false;

	EditorNode *editor_node = nullptr; // Our main editor node
	Viewport *xr_viewport = nullptr; // Viewport we render our XR content too

	VREditorAvatar *avatar = nullptr;

	Ref<Theme> theme;
	VRWindow *left_dock = nullptr;
	VRWindow *right_dock = nullptr;
	TabContainer *dock_slot[EditorNode::DOCK_SLOT_MAX];

public:
	static bool init_editor(EditorNode *p_editor_node, bool p_is_standalone);

	VREditor(EditorNode *p_editor_node, Viewport *p_xr_viewport, bool p_is_standalone);
	~VREditor();
};

#endif // VR_EDITOR_H
