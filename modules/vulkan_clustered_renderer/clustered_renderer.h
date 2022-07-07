/*************************************************************************/
/*  clustered_renderer.h                                                 */
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

#ifndef CLUSTERED_RENDERER_H
#define CLUSTERED_RENDERER_H

#include "servers/rendering/renderer_rd/renderer_scene_implementation_rd.h"

class ClusteredRenderer : public RendererRD::SceneImplementationRD {
public:
	// construction, initialisation and destruction
	virtual void init() override;

	// Geometry data
	virtual RID geometry_instance_create(RID p_base) override;
	virtual RendererRD::GeometryInstanceRD *get_geometry_instance(RID p_rid) override;

	// Data to render a frame
	virtual RendererRD::FrameData *create_framedata(const RendererSceneRender::CameraData *p_camera_data, const RendererSceneRender::CameraData *p_prev_camera_data) override;
	virtual RendererRD::RenderBuffers *create_renderbuffers() override;

	// Render function
	virtual void render_scene() override;

protected:
private:
};

#endif // !CLUSTERED_RENDERER_H
