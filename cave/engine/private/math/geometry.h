#pragma once
#include "engine/assets/mesh_asset.h"

namespace cave {

void BoxWireFrameHelper(const Vector3f& p_min,
                        const Vector3f& p_max,
                        std::vector<Vector3f>& p_out_positions,
                        std::vector<uint32_t>& p_out_indices);

MeshAsset MakeCubeMesh(const std::array<Vector3f, 8>& p_points);

MeshAsset MakeTetrahedronMesh(float p_size = 0.5f);

// @TODO: refactor the following
MeshAsset MakeGrassBillboard(const Vector3f& p_scale = Vector3f(0.5f));
MeshAsset MakeBoxMesh(float p_size = 0.5f);
MeshAsset MakeBoxWireframeMesh(float p_size = 0.5f);

MeshAsset MakeSkyBoxMesh();

}  // namespace cave
