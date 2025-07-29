#pragma once
#include "thumbnail_dvars.h"

#include "engine/assets/mesh_asset.h"

#include "modules/sw/pbr_pipeline.h"
#include "modules/sw/sampler.h"
#include "modules/sw/sw_renderer.h"

namespace cave::thumbnail {

std::vector<Color> Convert(const std::vector<Vector4f>& p_buffer, bool p_to_bgra = false);

void DrawMesh(const GpuMesh* p_mesh, IGraphicsManager& p_graphics_manager);

void FillDefaultMaterial(MaterialConstantBuffer& p_out);

}  // namespace cave::thumbnail
