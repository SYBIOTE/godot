/*************************************************************************/
/*  vr_editor_avatar.h                                                   */
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

#ifndef VR_EDITOR_AVATAR_H
#define VR_EDITOR_AVATAR_H

#include "vr_keyboard.h"
#include "vr_window.h"

#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/xr_nodes.h"
#include "scene/resources/material.h"

class VRPoke : public Node3D {
	GDCLASS(VRPoke, Node3D);

	// TODO add class for doing hits, now that we do raycast we can't do what we currently do.

private:
	float radius = 0.01;
	bool ray_enabled = true;

	Color color = Color(0.4, 0.4, 1.0, 0.5);

	Ref<StandardMaterial3D> material;
	MeshInstance3D *sphere = nullptr;
	MeshInstance3D *cast = nullptr;

protected:
public:
	void set_ray_enabled(bool p_enabled);
	bool get_ray_enabled() const { return ray_enabled; }

	bool is_select();

	VRPoke();
	~VRPoke();
};

class VRHand : public XRController3D {
	GDCLASS(VRHand, XRController3D);

private:
	VRPoke *poke = nullptr;

protected:
public:
	enum Hands {
		HAND_LEFT,
		HAND_RIGHT
	};

	VRPoke *get_poke() const { return poke; }
	void set_ray_enabled(bool p_enabled) { poke->set_ray_enabled(p_enabled); }

	VRHand(Hands p_hand);
	~VRHand();
};

class VREditorAvatar : public XROrigin3D {
	GDCLASS(VREditorAvatar, XROrigin3D);

private:
	enum {
		DELTA_HISTORY = 20
	};

	XRCamera3D *camera = nullptr;
	bool camera_is_tracking = false;

	Node3D *hud_pivot = nullptr;
	Node3D *hud_root = nullptr;
	real_t hud_offset = 0.0; // offset from eye height of our hud
	real_t hud_distance = 0.5; // desired distance of our hud to our player
	bool hud_moving = true; // we are adjusting the position of the hud
	bool hud_centering = true; // we are adjusting the rotation of the hud

	VRHand::Hands ray_active_on_hand = VRHand::HAND_RIGHT;
	VRHand *left_hand = nullptr;
	VRHand *right_hand = nullptr;
	void set_ray_active_on_hand(VRHand::Hands p_hand);
	void _on_button_pressed_on_hand(const String p_action, int p_hand);

	VRKeyboard *keyboard = nullptr;

	Vector<VRWindow *> windows;
	Vector<VRPoke *> pokes;

	// Performance info
	bool show_performance_info = true;
	double delta_history[DELTA_HISTORY];
	int delta_history_index = 0;
	Label3D *fps_label = nullptr;

protected:
	void _notification(int p_notification);

public:
	Node3D *get_hud_root() const { return hud_root; }

	real_t get_hud_offset() const { return hud_offset; }
	void set_hud_offset(real_t p_offset);

	real_t get_hud_distance() const { return hud_distance; }
	void set_hud_distance(real_t p_distance);

	void add_window_to_hud(VRWindow *p_window);
	void remove_window_from_hud(VRWindow *p_window);

	VREditorAvatar();
	~VREditorAvatar();
};

#endif //VR_EDITOR_AVATAR_H
