/*************************************************************************/
/*  vr_window.h                                                          */
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

#ifndef VR_WINDOW_H
#define VR_WINDOW_H

#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/main/viewport.h"

class VRWindow : public Node3D {
	GDCLASS(VRWindow, Node3D);

private:
	Size2 viewport_size; // original viewport size
	real_t viewport_scale = 0.00075; // viewport 2D size to 3D size ratio

	float press_distance = 0.01; // distance from Window to finger where we assume the finger is pressing on the screen (i.e. click)

	Vector2 last_poke_position;
	int last_poke_device = -1;

	void _init(Viewport *p_viewport);

protected:
	Viewport *viewport = nullptr; // viewport to which we render our 2D content
	Ref<StandardMaterial3D> material; // material used to draw our window
	MeshInstance3D *mesh_instance = nullptr; // mesh instance used to show our window

	virtual void input(const Ref<InputEvent> &p_event) override;

public:
	Viewport *get_scene_root() { return viewport; }

	bool ray_and_poke(int p_device, Vector3 p_global_position, Vector3 p_global_direction, bool p_select);

	VRWindow(Size2i p_viewport_size, real_t p_viewport_scale = 0.00075);
	VRWindow(Viewport *p_viewport, real_t p_viewport_scale = 0.00075);
	~VRWindow();
};

#endif VR_WINDOW_H
