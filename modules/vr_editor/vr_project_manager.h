/*************************************************************************/
/*  vr_project_manager.h                                                 */
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

#ifndef VR_PROJECT_MANAGER_H
#define VR_PROJECT_MANAGER_H

#include "scene/main/node.h"
#include "scene/main/viewport.h"
#include "vr_editor_avatar.h"
#include "vr_window.h"

class VRProjectManager : public Node {
	GDCLASS(VRProjectManager, Node);

private:
	bool is_standalone = false;

	Viewport *xr_viewport = nullptr; // Viewport we render our XR content too
	VRWindow *pm_window = nullptr;

	VREditorAvatar *avatar = nullptr;

protected:
	virtual void input(const Ref<InputEvent> &p_event) override;

public:
	static bool init_project_manager(SceneTree *p_scene_tree, bool p_is_standalone);

	VRProjectManager(Viewport *p_xr_viewport, bool p_is_standalone);
	~VRProjectManager();
};

#endif // VR_PROJECT_MANAGER_H