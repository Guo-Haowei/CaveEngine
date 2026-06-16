#pragma once
#include "engine/private/runtime/assets/MeshAsset.h"

// @TODO: this is coupled with MeshAsset, BAD!
namespace cave::math {

void BoxWireFrameHelper(const Vec3f& p_min,
                        const Vec3f& p_max,
                        std::vector<Vec3f>& p_out_positions,
                        std::vector<uint32_t>& p_out_indices);

MeshAsset MakeCubeMesh(const std::array<Vec3f, 8>& p_points);

MeshAsset MakeTetrahedronMesh(float p_size = 0.5f);

// @TODO: refactor the following
MeshAsset MakeGrassBillboard(const Vec3f& p_scale = Vec3f(0.5f));
MeshAsset MakeBoxMesh(float p_size = 0.5f);
MeshAsset MakeBoxWireframeMesh(float p_size = 0.5f);

MeshAsset MakeSkyBoxMesh();

}  // namespace cave::math
