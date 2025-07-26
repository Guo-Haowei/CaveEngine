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

static std::shared_ptr<MeshAsset> CreateCubeMesh(const Vector3f& p_scale) {
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

static std::shared_ptr<MeshAsset> CreateSphereMesh(float p_radius,
                                                   int p_rings = 60,
                                                   int p_sectors = 60) {
    auto mesh = std::make_shared<MeshAsset>();

    auto& indices = mesh->indices;
    constexpr float pi = glm::pi<float>();
    for (int step_x = 0; step_x <= p_sectors; ++step_x) {
        for (int step_y = 0; step_y <= p_rings; ++step_y) {
            const float x_seg = (float)step_x / (float)p_sectors;
            const float y_seg = (float)step_y / (float)p_rings;
            const Vector3f normal{
                std::cos(x_seg * 2.0f * pi) * std::sin(y_seg * pi),
                std::cos(y_seg * pi),
                std::sin(x_seg * 2.0f * pi) * std::sin(y_seg * pi)
            };

            mesh->positions.emplace_back(p_radius * normal);
            mesh->normals.emplace_back(normal);
            mesh->texcoords_0.emplace_back(Vector2f(1.0f - x_seg, y_seg));
        }
    }

    for (int y = 0; y < p_rings; ++y) {
        for (int x = 0; x < p_sectors; ++x) {
            /* a - b
               |   |
               c - d */
            uint32_t a = (y * (p_sectors + 1) + x);
            uint32_t b = (y * (p_sectors + 1) + x + 1);
            uint32_t c = ((y + 1) * (p_sectors + 1) + x);
            uint32_t d = ((y + 1) * (p_sectors + 1) + x + 1);

            indices.emplace_back(a);
            indices.emplace_back(c);
            indices.emplace_back(b);

            indices.emplace_back(b);
            indices.emplace_back(c);
            indices.emplace_back(d);
        }
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(indices.size());
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

static std::shared_ptr<MeshAsset> CreateCylinderMesh(float p_radius,
                                                     float p_height,
                                                     int p_sectors = 60,
                                                     int p_height_sector = 1) {
    auto mesh = std::make_shared<MeshAsset>();

    auto& indices = mesh->indices;
    constexpr float pi = glm::pi<float>();

    std::array<float, 2> heights = { 0.5f * p_height, -0.5f * p_height };

    // cylinder side
    float height_step = (float)p_height / p_height_sector;
    for (float y = heights[1]; y < heights[0]; y += height_step) {
        uint32_t point_offset = (uint32_t)mesh->positions.size();
        for (int index = 0; index < p_sectors; ++index) {
            float angle_1 = 2.0f * pi * index / p_sectors;
            float x_1 = p_radius * std::cos(angle_1);
            float z_1 = p_radius * std::sin(angle_1);
            float angle_2 = 2.0f * pi * (index + 1) / p_sectors;
            float x_2 = p_radius * std::cos(angle_2);
            float z_2 = p_radius * std::sin(angle_2);

            Vector3f point_1(x_1, y, z_1);
            Vector3f point_2(x_1, y + height_step, z_1);

            Vector3f point_3(x_2, y, z_2);
            Vector3f point_4(x_2, y + height_step, z_2);

            Vector3f AB = point_1 - point_2;
            Vector3f AC = point_1 - point_3;
            Vector3f normal = normalize(cross(AB, AC));

            mesh->positions.emplace_back(point_1);
            mesh->positions.emplace_back(point_2);
            mesh->positions.emplace_back(point_3);
            mesh->positions.emplace_back(point_4);

            mesh->normals.emplace_back(normal);
            mesh->normals.emplace_back(normal);
            mesh->normals.emplace_back(normal);
            mesh->normals.emplace_back(normal);

            float u0 = 1.0f - static_cast<float>(index) / p_sectors;
            float u1 = 1.0f - static_cast<float>(index + 1) / p_sectors;
            float v0 = 1.0f - (y - heights[1]) / p_height;
            float v1 = 1.0f - (y + height_step - heights[1]) / p_height;

            mesh->texcoords_0.emplace_back(Vector2f(u0, v0));  // point_1
            mesh->texcoords_0.emplace_back(Vector2f(u0, v1));  // point_2
            mesh->texcoords_0.emplace_back(Vector2f(u1, v0));  // point_3
            mesh->texcoords_0.emplace_back(Vector2f(u1, v1));  // point_4

            const uint32_t a = point_offset + 4 * index;
            const uint32_t c = point_offset + 4 * index + 1;
            const uint32_t b = point_offset + 4 * index + 2;
            [[maybe_unused]] const uint32_t d = point_offset + 4 * index + 3;

            indices.emplace_back(a);
            indices.emplace_back(c);
            indices.emplace_back(b);

            indices.emplace_back(c);
            indices.emplace_back(d);
            indices.emplace_back(b);
        }
    }

    // cylinder circles
    for (float height : heights) {
        uint32_t offset = static_cast<uint32_t>(mesh->positions.size());

        Vector3f normal = normalize(Vector3f(0.0f, height, 0.0f));

        for (int index = 0; index <= p_sectors; ++index) {
            float angle = 2.0f * pi * index / p_sectors;
            float x = p_radius * glm::cos(angle);
            float z = p_radius * glm::sin(angle);

            Vector3f point(x, height, z);
            Vector2f uv(0.5f + 0.5f * x / p_radius,
                        0.5f + 0.5f * z / p_radius);


            mesh->positions.emplace_back(point);
            mesh->normals.emplace_back(normal);
            mesh->texcoords_0.emplace_back(uv);
        }

        mesh->positions.emplace_back(Vector3f(0.0f, height, 0.0f));
        mesh->normals.emplace_back(normal);
        mesh->texcoords_0.emplace_back(Vector2f(0.5f));

        uint32_t center_index = static_cast<uint32_t>(mesh->positions.size()) - 1;
        for (int index = 0; index < p_sectors; ++index) {
            if (height < 0) {
                indices.emplace_back(offset + index);
                indices.emplace_back(offset + index + 1);
                indices.emplace_back(center_index);
            } else {
                indices.emplace_back(offset + index + 1);
                indices.emplace_back(offset + index);
                indices.emplace_back(center_index);
            }
        }
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(indices.size());
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

static std::shared_ptr<MeshAsset> CreateConeMesh(float p_radius,
                                                 float p_height,
                                                 int p_sectors = 60) {
    auto mesh = std::make_shared<MeshAsset>();

    auto& indices = mesh->indices;
    constexpr float pi = glm::pi<float>();

    const float height_half = 0.5f * p_height;
    const Vector3f apex{ 0.0f, height_half, 0.0f };

    // cone side
    for (int index = 0; index < p_sectors; ++index) {
        const float angle_1 = 2.0f * pi * index / p_sectors;
        const float x_1 = p_radius * glm::cos(angle_1);
        const float z_1 = p_radius * glm::sin(angle_1);

        const float angle_2 = 2.0f * pi * (index + 1) / p_sectors;
        const float x_2 = p_radius * glm::cos(angle_2);
        const float z_2 = p_radius * glm::sin(angle_2);

        Vector3f point_1(x_1, -height_half, z_1);
        Vector3f point_2(x_2, -height_half, z_2);

        // Vector3f normal = glm::normalize(Vector3f(x, 0.0f, z));
        Vector3f AB = point_1 - apex;
        Vector3f AC = point_2 - apex;
        Vector3f normal = normalize(cross(AC, AB));

        mesh->positions.emplace_back(point_1);
        mesh->positions.emplace_back(apex);
        mesh->positions.emplace_back(point_2);

        mesh->normals.emplace_back(normal);
        mesh->normals.emplace_back(normal);
        mesh->normals.emplace_back(normal);

        mesh->indices.emplace_back(3 * index);
        mesh->indices.emplace_back(3 * index + 1);
        mesh->indices.emplace_back(3 * index + 2);

        // @TODO: fix dummy uv
        mesh->texcoords_0.emplace_back(Vector2f());
        mesh->texcoords_0.emplace_back(Vector2f());
        mesh->texcoords_0.emplace_back(Vector2f());
    }

#if 0
    for (int index = 0; index < p_sectors; ++index) {
        /* a - b
           |   |
           c - d */
        const uint32_t a = 2 * index;
        const uint32_t c = 2 * index + 1;
        const uint32_t b = 2 * index + 2;
        const uint32_t d = 2 * index + 3;

        indices.emplace_back(a);
        indices.emplace_back(b);
        indices.emplace_back(c);

        indices.emplace_back(c);
        indices.emplace_back(b);
        indices.emplace_back(d);
    }
#endif

    // cylinder circles
    {
        uint32_t offset = static_cast<uint32_t>(mesh->positions.size());

        Vector3f normal(0, -1, 0);

        for (int index = 0; index <= p_sectors; ++index) {
            float angle = 2.0f * pi * index / p_sectors;
            float x = p_radius * glm::cos(angle);
            float z = p_radius * glm::sin(angle);

            Vector3f point(x, -height_half, z);

            mesh->positions.emplace_back(point);
            mesh->normals.emplace_back(normal);
            mesh->texcoords_0.emplace_back(Vector2f());
        }

        // center
        mesh->positions.emplace_back(Vector3f(0.0f, -height_half, 0.0f));
        mesh->normals.emplace_back(normal);
        mesh->texcoords_0.emplace_back(Vector2f());

        uint32_t center_index = static_cast<uint32_t>(mesh->positions.size()) - 1;
        for (int index = 0; index < p_sectors; ++index) {
            indices.emplace_back(offset + index);
            indices.emplace_back(offset + index + 1);
            indices.emplace_back(center_index);
        }
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(indices.size());
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

static std::shared_ptr<MeshAsset> CreateTorusMesh(float p_radius,
                                                  float p_tube_radius = 0.2f,
                                                  int p_sectors = 60,
                                                  int p_tube_sectors = 60) {
    auto mesh = std::make_shared<MeshAsset>();

    constexpr float two_pi = 2.0f * glm::pi<float>();
    for (int index_1 = 0; index_1 <= p_sectors; ++index_1) {
        const float angle_1 = two_pi * index_1 / p_sectors;
        for (int index_2 = 0; index_2 <= p_tube_sectors; ++index_2) {
            const float angle_2 = two_pi * index_2 / p_tube_sectors;
            const float x = (p_radius + p_tube_radius * glm::cos(angle_2)) * glm::cos(angle_1);
            const float y = p_tube_radius * glm::sin(angle_2);
            const float z = (p_radius + p_tube_radius * glm::cos(angle_2)) * glm::sin(angle_1);
            const float nx = p_tube_radius * glm::cos(angle_2) * glm::cos(angle_1);
            const float ny = p_tube_radius * glm::sin(angle_2);
            const float nz = p_tube_radius * glm::cos(angle_2) * glm::sin(angle_1);

            Vector2f uv(1.0f - static_cast<float>(index_1) / p_sectors,
                        1.0f - static_cast<float>(index_2) / p_tube_sectors);

            mesh->positions.emplace_back(Vector3f(x, y, z));
            mesh->normals.emplace_back(normalize(Vector3f(nx, ny, nz)));
            mesh->texcoords_0.emplace_back(uv);
        }
    }

    auto& indices = mesh->indices;

    for (int index_1 = 0; index_1 < p_sectors; ++index_1) {
        for (int index_2 = 0; index_2 < p_tube_sectors; ++index_2) {
            /* a - b
               |   |
               c - d */
            const uint32_t a = index_2 + index_1 * (p_tube_sectors + 1);
            const uint32_t b = a + (p_tube_sectors + 1);
            const uint32_t c = a + 1;
            const uint32_t d = b + 1;

            indices.emplace_back(a);
            indices.emplace_back(c);
            indices.emplace_back(b);

            indices.emplace_back(b);
            indices.emplace_back(c);
            indices.emplace_back(d);
        }
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(indices.size());
    subset.index_offset = 0;
    mesh->subsets.emplace_back(subset);

    mesh->CreateRenderData();
    return mesh;
}

}  // namespace cave
