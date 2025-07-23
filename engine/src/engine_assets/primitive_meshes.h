#pragma once
#include "engine/assets/mesh_asset.h"
#include "engine/math/matrix_transform.h"

namespace cave {

/**
 *        E__________________ H
 *       /|                 /|
 *      / |                / |
 *     /  |               /  |
 *   A/___|______________/D  |
 *    |   |              |   |
 *    |   |              |   |
 *    |   |              |   |
 *    |  F|______________|___|G
 *    |  /               |  /
 *    | /                | /
 *   B|/_________________|C
 *
 */

// clang-format off
enum { A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7 };
// clang-format on

static std::shared_ptr<MeshAsset> CreatePlaneMesh(const Vector3f& p_point_0,
                                                  const Vector3f& p_point_1,
                                                  const Vector3f& p_point_2,
                                                  const Vector3f& p_point_3) {
    auto mesh = std::make_shared<MeshAsset>();
    mesh->positions = {
        p_point_0,
        p_point_1,
        p_point_2,
        p_point_3,
    };

    Vector3f normal = cross(p_point_0 - p_point_1, p_point_0 - p_point_2);
    normal = normalize(normal);

    mesh->normals = {
        normal,
        normal,
        normal,
        normal,
    };

    mesh->texcoords_0 = {
        Vector2f(0, 1),  // top-left
        Vector2f(0, 0),  // bottom-left
        Vector2f(1, 0),  // bottom-right
        Vector2f(1, 1),  // top-right
    };

    // clang-format off
    mesh->indices = {
#if 1
        A, B, D,  // ABD
        D, B, C,  // DBC
#else
        A, D, B, // ADB
        D, C, B, // DBC
#endif
    };
    // clang-format on

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(mesh->indices.size());
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

static std::shared_ptr<MeshAsset> CreatePlaneMesh(const Vector3f& p_scale) {
    const float x = p_scale.x;
    const float y = p_scale.y;
    Vector3f a(-x, +y, 0.0f);  // A
    Vector3f b(-x, -y, 0.0f);  // B
    Vector3f c(+x, -y, 0.0f);  // C
    Vector3f d(+x, +y, 0.0f);  // D
    return CreatePlaneMesh(a, b, c, d);
}

[[maybe_unused]] static std::shared_ptr<MeshAsset> CreateCubeMesh(const Vector3f& p_scale = Vector3f(0.5f)) {
    auto mesh = std::make_shared<MeshAsset>();
    // clang-format off
    constexpr uint32_t indices[] = {
        0,          1,          2,          0,          2,          3,
        0 + 4,      2 + 4,      1 + 4,      0 + 4,      3 + 4,      2 + 4,  // swapped winding
        0 + 4 * 2,  1 + 4 * 2,  2 + 4 * 2,  0 + 4 * 2,  2 + 4 * 2,  3 + 4 * 2,
        0 + 4 * 3,  2 + 4 * 3,  1 + 4 * 3,  0 + 4 * 3,  3 + 4 * 3,  2 + 4 * 3, // swapped winding
        0 + 4 * 4,  2 + 4 * 4,  1 + 4 * 4,  0 + 4 * 4,  3 + 4 * 4,  2 + 4 * 4, // swapped winding
        0 + 4 * 5,  1 + 4 * 5,  2 + 4 * 5,  0 + 4 * 5,  2 + 4 * 5,  3 + 4 * 5,
    };
    // clang-format on

    const Vector3f& s = p_scale;
    mesh->positions = {
        // -Z
        Vector3f(-s.x, +s.y, -s.z),
        Vector3f(-s.x, -s.y, -s.z),
        Vector3f(+s.x, -s.y, -s.z),
        Vector3f(+s.x, +s.y, -s.z),

        // +Z
        Vector3f(-s.x, +s.y, +s.z),
        Vector3f(-s.x, -s.y, +s.z),
        Vector3f(+s.x, -s.y, +s.z),
        Vector3f(+s.x, +s.y, +s.z),

        // -X
        Vector3f(-s.x, -s.y, +s.z),
        Vector3f(-s.x, -s.y, -s.z),
        Vector3f(-s.x, +s.y, -s.z),
        Vector3f(-s.x, +s.y, +s.z),

        // +X
        Vector3f(+s.x, -s.y, +s.z),
        Vector3f(+s.x, -s.y, -s.z),
        Vector3f(+s.x, +s.y, -s.z),
        Vector3f(+s.x, +s.y, +s.z),

        // -Y
        Vector3f(-s.x, -s.y, +s.z),
        Vector3f(-s.x, -s.y, -s.z),
        Vector3f(+s.x, -s.y, -s.z),
        Vector3f(+s.x, -s.y, +s.z),

        // +Y
        Vector3f(-s.x, +s.y, +s.z),
        Vector3f(-s.x, +s.y, -s.z),
        Vector3f(+s.x, +s.y, -s.z),
        Vector3f(+s.x, +s.y, +s.z),
    };

    mesh->texcoords_0 = {
        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),

        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),

        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),

        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),

        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),

        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        Vector2f(1, 0),
    };

    mesh->normals = {
        Vector3f(0, 0, -1),
        Vector3f(0, 0, -1),
        Vector3f(0, 0, -1),
        Vector3f(0, 0, -1),

        Vector3f(0, 0, 1),
        Vector3f(0, 0, 1),
        Vector3f(0, 0, 1),
        Vector3f(0, 0, 1),

        Vector3f(-1, 0, 0),
        Vector3f(-1, 0, 0),
        Vector3f(-1, 0, 0),
        Vector3f(-1, 0, 0),

        Vector3f(1, 0, 0),
        Vector3f(1, 0, 0),
        Vector3f(1, 0, 0),
        Vector3f(1, 0, 0),

        Vector3f(0, -1, 0),
        Vector3f(0, -1, 0),
        Vector3f(0, -1, 0),
        Vector3f(0, -1, 0),

        Vector3f(0, 1, 0),
        Vector3f(0, 1, 0),
        Vector3f(0, 1, 0),
        Vector3f(0, 1, 0),
    };

    for (int i = 0; i < array_length(indices); i += 3) {
        mesh->indices.emplace_back(indices[i]);
        mesh->indices.emplace_back(indices[i + 2]);
        mesh->indices.emplace_back(indices[i + 1]);
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = array_length(indices);
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

}  // namespace cave
