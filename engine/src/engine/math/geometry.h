#pragma once
#include "engine/assets/mesh_asset.h"

namespace cave {

void BoxWireFrameHelper(const Vector3f& p_min,
                        const Vector3f& p_max,
                        std::vector<Vector3f>& p_out_positions,
                        std::vector<uint32_t>& p_out_indices);

MeshAsset MakeCubeMesh(const Vector3f& p_scale = Vector3f(0.5f));

MeshAsset MakeCubeMesh(const std::array<Vector3f, 8>& p_points);

MeshAsset MakeTetrahedronMesh(float p_size = 0.5f);

MeshAsset MakeSphereMesh(float p_radius,
                         int p_rings = 60,
                         int p_sectors = 60);

MeshAsset MakeCylinderMesh(float p_radius,
                           float p_height,
                           int p_sectors = 60,
                           int p_height_sector = 1);

MeshAsset MakeConeMesh(float p_radius,
                       float p_height,
                       int p_sectors = 60);

MeshAsset MakeTorusMesh(float p_radius,
                        float p_tube_radius,
                        int p_sectors = 60,
                        int p_tube_sectors = 60);

// @TODO: refactor the following
MeshAsset MakeGrassBillboard(const Vector3f& p_scale = Vector3f(0.5f));
MeshAsset MakeBoxMesh(float p_size = 0.5f);
MeshAsset MakeBoxWireframeMesh(float p_size = 0.5f);

MeshAsset MakeSkyBoxMesh();

}  // namespace cave
