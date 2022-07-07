/*************************************************************************/
/*  clustered_renderer.cpp                                               */
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

#include "clustered_renderer.h"

// ClusteredRenderer
void ClusteredRenderer::init() {
}

// Geometry data
RID ClusteredRenderer::geometry_instance_create(RID p_base) {
	// TODO implement
	return RID();
}

RendererRD::GeometryInstanceRD *ClusteredRenderer::get_geometry_instance(RID p_rid) {
	// TODO implement
	return nullptr;
}

// Data to render a frame
RendererRD::FrameData *ClusteredRenderer::create_framedata(const RendererSceneRender::CameraData *p_camera_data, const RendererSceneRender::CameraData *p_prev_camera_data) {
	// TODO implement
	return nullptr;
}

RendererRD::RenderBuffers *ClusteredRenderer::create_renderbuffers() {
	// TODO implement
	return nullptr;
}

// Render function
void ClusteredRenderer::render_scene() {
	// TODO implement
}
