/*************************************************************************/
/*  renderer_scene_implementation_rd.h                                   */
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

#ifndef RENDERING_SCENE_IMPLEMENTATION_RD_H
#define RENDERING_SCENE_IMPLEMENTATION_RD_H

#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering/renderer_scene_render.h"
#include "servers/rendering/rendering_device.h"

namespace RendererRD {

// LightInstanceRD is am implementation of LightInstance
// !BAS! To consider, adding these methods to LightInstance and turning this into an object that can be extended in script/extension
struct LightInstanceRD : RendererSceneRender::LightInstance {
};

// GeometryInstanceRD is an implementation of GeometryInstance
// !BAS! To consider, adding these methods to GeometryInstance and turning this into an object that can be extended in script/extension
struct GeometryInstanceRD : RendererSceneRender::GeometryInstance {
	virtual void set_skeleton(RID p_skeleton) = 0;
	virtual void set_material_override(RID p_override) = 0;
	virtual void material_overlay(RID p_override) = 0;
	virtual void set_surface_materials(const Vector<RID> &p_material) = 0;
	virtual void set_mesh_instance(RID p_mesh_instance) = 0;
	virtual void set_transform(const Transform3D &p_transform, const AABB &p_aabb, const AABB &p_transformed_aabbb) = 0;
	virtual void set_layer_mask(uint32_t p_layer_mask) = 0;
	virtual void set_lod_bias(float p_lod_bias) = 0;
	virtual void set_transparency(float p_transparency) = 0;
	virtual void set_fade_range(bool p_enable_near, float p_near_begin, float p_near_end, bool p_enable_far, float p_far_begin, float p_far_end) = 0;
	virtual void set_parent_fade_alpha(float p_alpha) = 0;
	virtual void set_use_baked_light(bool p_enable) = 0;
	virtual void set_use_dynamic_gi(bool p_enable) = 0;
	virtual void set_use_lightmap(RID p_lightmap_instance, const Rect2 &p_lightmap_uv_scale, int p_lightmap_slice_index) = 0;
	virtual void set_lightmap_capture(const Color *p_sh9) = 0;
	virtual void set_instance_shader_parameters_offset(int32_t p_offset) = 0;
	virtual void set_cast_double_sided_shadows(bool p_enable) = 0;
};

// FrameData stores information about our frame such as matrices and view info
// Can be inherited by our render implementation to add additional information
class FrameData {
public:
	//////////////////////////////////////////////////////////////////////////
	// Camera and projection info

	// !BAS! should we embed two instances of CameraData in here (current and previous), might be easier?

	// General view info
	uint32_t view_count = 1;
	float z_near = 0.0;
	float z_far = 0.0;
	bool cam_orthogonal = false;

	// Combined camera transform and projection matrix
	Transform3D cam_transform;
	CameraMatrix cam_projection;
	Vector2 taa_jitter;

	// Per view offset and projection matrices
	Vector3 view_eye_offset[RendererSceneRender::MAX_RENDER_VIEWS];
	CameraMatrix view_projection[RendererSceneRender::MAX_RENDER_VIEWS];

	// Previous frame transform data
	Transform3D prev_cam_transform;
	CameraMatrix prev_cam_projection;
	Vector2 prev_taa_jitter;
	Vector3 prev_view_eye_offset[RendererSceneRender::MAX_RENDER_VIEWS];
	CameraMatrix prev_view_projection[RendererSceneRender::MAX_RENDER_VIEWS];

	//////////////////////////////////////////////////////////////////////////
	// Scene data

	const PagedArray<RendererSceneRender::GeometryInstance *> *instances = nullptr;
	const PagedArray<RID> *lights = nullptr;
	const PagedArray<RID> *reflection_probes = nullptr;
	const PagedArray<RID> *voxel_gi_instances = nullptr;
	const PagedArray<RID> *decals = nullptr;
	const PagedArray<RID> *lightmaps = nullptr;
	const PagedArray<RID> *fog_volumes = nullptr;

	// We probably need to add more here.

	virtual ~FrameData();
	virtual void cleanup();

	// We're splitting our scene_data uniform into two:
	// - frame_data that contains data that is shared amongst implementations and can thus also be used by effects
	// - implementation_data that contains implementation specific data.
	// if we can find a way to make frame_data extendable through including an implementation specific glsl file
	// that would be even nicer.

	struct UBOs {
		RID frame_data;
		RID implementation_data;

		void cleanup();
	};

	virtual void update_uniform_buffers(UBOs &p_frame_data_rids); // updates frame_data, override to also update implementation_data

protected:
private:
};

// RenderBuffers stores information about the buffers we use to render too
// Can be inherited by our render implementation to add additional information
class RenderBuffers {
public:
	virtual ~RenderBuffers();
	virtual void cleanup();

protected:
private:
};

// SceneImplementationRD is our base class for RenderDevice based render implementations
class SceneImplementationRD {
public:
	// We register scene implementations
	struct Info {
		String description;
		SceneImplementationRD *(*creation_func)() = nullptr;
	};

	template <class T>
	static SceneImplementationRD *creator() {
		return memnew(T);
	}

	static HashMap<StringName, Info> scene_implementations;
	static StringName default_scene_implementation;

	template <class T>
	static void register_scene_implementation(const StringName p_class, const String p_description) {
		ERR_FAIL_COND_MSG(scene_implementations.has(p_class), "Scene implementation '" + String(p_class) + "' already exists.");
		scene_implementations[p_class] = RendererRD::SceneImplementationRD::Info();
		RendererRD::SceneImplementationRD::Info &t = scene_implementations[p_class];
		t.description = p_description;
		t.creation_func = &creator<T>;

		print_line(String("Register ") + String(p_class));

		if (default_scene_implementation == StringName()) {
			default_scene_implementation = p_class;
		}
	}

	// construction, initialisation and destruction
	static SceneImplementationRD *create_scene_implementation(const StringName p_class = StringName()) {
		if (p_class == StringName() && scene_implementations.has(default_scene_implementation)) {
			return scene_implementations[default_scene_implementation].creation_func();
		} else if (scene_implementations.has(p_class)) {
			return scene_implementations[p_class].creation_func();
		} else {
			return nullptr;
		}
	}
	virtual ~SceneImplementationRD();
	virtual void init() = 0;
	virtual void cleanup();

	// Geometry data
	virtual RID geometry_instance_create(RID p_base) = 0;
	virtual GeometryInstanceRD *get_geometry_instance(RID p_rid) = 0;

	// Data to render a frame
	virtual FrameData *create_framedata(const RendererSceneRender::CameraData *p_camera_data, const RendererSceneRender::CameraData *p_prev_camera_data) = 0;
	virtual RenderBuffers *create_renderbuffers() = 0;

	// Render function
	virtual void render_scene() = 0;

protected:
	FrameData::RIDs frame_data_rids;

private:
};

} // namespace RendererRD

#define GDREGISTER_SCENE_IMPLEMENTATION(m_class, m_description) \
	RendererRD::SceneImplementationRD::register_scene_implementation<m_class>(StringName(#m_class), String(m_description));

#endif // !RENDERING_SCENE_IMPLEMENTATION_RD_H
