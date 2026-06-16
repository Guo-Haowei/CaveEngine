#include "Geometry.h"

#include "engine/private/core/math/MatrixTransform.h"

namespace cave::math {

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

MeshAsset MakeCubeMesh(const std::array<Vec3f, 8>& p_points) {
    MeshAsset mesh;

    const Vec3f& a = p_points[A];
    const Vec3f& b = p_points[B];
    const Vec3f& c = p_points[C];
    const Vec3f& d = p_points[D];
    const Vec3f& e = p_points[E];
    const Vec3f& f = p_points[F];
    const Vec3f& g = p_points[G];
    const Vec3f& h = p_points[H];
    // clang-format off
    mesh.positions = {
        // front
        a, b, c, d,
        // back
        e, h, g ,f,
        // left
        a, e, f, b,
        // right
        c, g, h, d,
        // up
        a, d, h, e,
        // down
        b, f, g, c,
    };
    // clang-format on

    mesh.indices.clear();
    mesh.normals.clear();
    for (int i = 0; i < 24; i += 4) {
        const Vec3f& A = mesh.positions[i];
        const Vec3f& B = mesh.positions[i + 1];
        const Vec3f& C = mesh.positions[i + 2];
        // const Vector3f& D = mesh.positions[i + 3];
        Vec3f AB = B - A;
        Vec3f AC = C - A;
        Vec3f normal = normalize(cross(AB, AC));

        mesh.normals.emplace_back(normal);
        mesh.normals.emplace_back(normal);
        mesh.normals.emplace_back(normal);
        mesh.normals.emplace_back(normal);

        mesh.indices.emplace_back(i);
        mesh.indices.emplace_back(i + 1);
        mesh.indices.emplace_back(i + 2);

        mesh.indices.emplace_back(i);
        mesh.indices.emplace_back(i + 2);
        mesh.indices.emplace_back(i + 3);

        // TODO: fix dummy uv
        mesh.texcoords_0.push_back(Vec2f::Zero);
        mesh.texcoords_0.push_back(Vec2f::Zero);
        mesh.texcoords_0.push_back(Vec2f::Zero);
        mesh.texcoords_0.push_back(Vec2f::Zero);
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(mesh.indices.size());
    subset.index_offset = 0;
    mesh.subsets.emplace_back(subset);

    mesh.CreateRenderData();
    return mesh;
}

MeshAsset MakeTetrahedronMesh(float p_size) {
    // Vertex data for a tetrahedron (4 vertices, each with x, y, z coordinates)
    constexpr float h = 2.0f / 2.449f;
    Vec3f vertices[] = {
        p_size * Vec3f(+0, +h, +1),  // top front
        p_size * Vec3f(+0, +h, -1),  // top back
        p_size * Vec3f(-1, -h, +0),  // bottom left vertex
        p_size * Vec3f(+1, -h, +0),  // bottom right vertex
    };

    static const uint32_t indices[] = {
        0, 2, 3,  // face 1
        0, 3, 1,  // face 2
        0, 1, 2,  // face 3
        1, 3, 2,  // face 4
    };

    MeshAsset mesh;

    for (uint32_t i = 0; i < std::size(indices); i += 3) {
        Vec3f A = vertices[indices[i]];
        Vec3f B = vertices[indices[i + 1]];
        Vec3f C = vertices[indices[i + 2]];

        Vec3f normal = normalize(cross(A - B, A - C));

        mesh.positions.emplace_back(A);
        mesh.positions.emplace_back(B);
        mesh.positions.emplace_back(C);

        mesh.normals.emplace_back(normal);
        mesh.normals.emplace_back(normal);
        mesh.normals.emplace_back(normal);

        mesh.indices.emplace_back(i);
        mesh.indices.emplace_back(i + 1);
        mesh.indices.emplace_back(i + 2);

        mesh.texcoords_0.emplace_back(Vec2f());
        mesh.texcoords_0.emplace_back(Vec2f());
        mesh.texcoords_0.emplace_back(Vec2f());
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(mesh.indices.size());
    subset.index_offset = 0;
    mesh.subsets.emplace_back(subset);

    mesh.CreateRenderData();
    return mesh;
}

void BoxWireFrameHelper(const Vec3f& p_min,
                        const Vec3f& p_max,
                        std::vector<Vec3f>& p_out_positions,
                        std::vector<uint32_t>& p_out_indices) {
    p_out_positions = {
        Vec3f(p_min.x, p_max.y, p_max.z),  // A
        Vec3f(p_min.x, p_min.y, p_max.z),  // B
        Vec3f(p_max.x, p_min.y, p_max.z),  // C
        Vec3f(p_max.x, p_max.y, p_max.z),  // D
        Vec3f(p_min.x, p_max.y, p_min.z),  // E
        Vec3f(p_min.x, p_min.y, p_min.z),  // F
        Vec3f(p_max.x, p_min.y, p_min.z),  // G
        Vec3f(p_max.x, p_max.y, p_min.z),  // H
    };

    p_out_indices = {
        A, B, D,  // ABD
        D, B, C,  // DBC
        E, H, F,  // EHF
        H, G, F,  // HGF
        D, C, G,  // DCG
        D, G, H,  // DGH
        A, F, B,  // AFB
        A, E, F,  // AEF
        A, D, H,  // ADH
        A, H, E,  // AHE
        B, F, G,  // BFG
        B, G, C,  // BGC
    };
}

MeshAsset MakeBoxMesh(float size) {
    MeshAsset mesh;
    Vec3f min(-size);
    Vec3f max(+size);
    BoxWireFrameHelper(min, max, mesh.positions, mesh.indices);
    mesh.CreateRenderData();
    return mesh;
}

MeshAsset MakeGrassBillboard(const Vec3f& p_scale) {
    MeshAsset mesh;

    const float x = p_scale.x;
    const float y = p_scale.y;

    std::array<Vec4f, 4> points = {
        Vec4f(-x, 2 * y, 0.0f, 1.0f),  // A
        Vec4f(-x, 0.0f, 0.0f, 1.0f),   // B
        Vec4f(+x, 0.0f, 0.0f, 1.0f),   // C
        Vec4f(+x, 2 * y, 0.0f, 1.0f),  // D
    };

    // @TODO: correct sampler
    constexpr std::array<Vec2f, 4> uvs = {
        Vec2f(0, 1),  // top-left
        Vec2f(0, 0),  // bottom-left
        Vec2f(1, 0),  // bottom-right
        Vec2f(1, 1),  // top-right
    };

    constexpr uint32_t indices[] = {
        A, B, D,  // ABD
        D, B, C,  // DBC
    };

    Degree angle;
    for (int i = 0; i < 3; ++i, angle += Degree(120.0f)) {
        const Matrix4x4f rotation = Rotate(angle, Vec3f(0, 1, 0));
        const Vec4f normal4 = rotation * Vec4f{ 0, 0, 1, 0 };
        const Vec3f normal = normal4.xyz;

        uint32_t offset = static_cast<uint32_t>(mesh.positions.size());
        for (size_t j = 0; j < points.size(); ++j) {
            Vec4f tmp = rotation * points[i];
            mesh.positions.emplace_back(Vec3f(tmp.xyz));
            mesh.normals.emplace_back(normal);
            mesh.texcoords_0.emplace_back(uvs[j]);
        }

        for (size_t j = 0; j < std::size(indices); ++j) {
            mesh.indices.emplace_back(indices[j] + offset);
        }
    }

    // flip uv
    for (auto& uv : mesh.texcoords_0) {
        uv.y = 1.0f - uv.y;
    }

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(mesh.indices.size());
    subset.index_offset = 0;
    mesh.subsets.emplace_back(subset);

    mesh.CreateRenderData();
    return mesh;
}

MeshAsset MakeSkyBoxMesh() {
    float size = 1.0f;
    MeshAsset mesh;
    mesh.positions = {
        Vec3f(-size, +size, +size),  // A
        Vec3f(-size, -size, +size),  // B
        Vec3f(+size, -size, +size),  // C
        Vec3f(+size, +size, +size),  // D
        Vec3f(-size, +size, -size),  // E
        Vec3f(-size, -size, -size),  // F
        Vec3f(+size, -size, -size),  // G
        Vec3f(+size, +size, -size),  // H
    };

    mesh.indices = {
        A, D, B,  // ABD
        D, C, B,  // DBC
        E, F, H,  // EHF
        H, F, G,  // HGF
        D, G, C,  // DCG
        D, H, G,  // DGH
        A, B, F,  // AFB
        A, F, E,  // AEF
        A, H, D,  // ADH
        A, E, H,  // AHE
        B, G, F,  // BFG
        B, C, G,  // BGC
        // A, B, D,  // ABD
        // D, B, C,  // DBC
        // E, H, F,  // EHF
        // H, G, F,  // HGF
        // D, C, G,  // DCG
        // D, G, H,  // DGH
        // A, F, B,  // AFB
        // A, E, F,  // AEF
        // A, D, H,  // ADH
        // A, H, E,  // AHE
        // B, F, G,  // BFG
        // B, G, C,  // BGC
    };

    MeshAsset::MeshSubset subset;
    subset.index_count = static_cast<uint32_t>(mesh.indices.size());
    subset.index_offset = 0;
    mesh.subsets.emplace_back(subset);

    mesh.CreateRenderData();
    return mesh;
}

// load scene
MeshAsset MakeBoxWireframeMesh(float size) {
    MeshAsset mesh;
    mesh.positions = {
        Vec3f(-size, +size, +size),  // A
        Vec3f(-size, -size, +size),  // B
        Vec3f(+size, -size, +size),  // C
        Vec3f(+size, +size, +size),  // D
        Vec3f(-size, +size, -size),  // E
        Vec3f(-size, -size, -size),  // F
        Vec3f(+size, -size, -size),  // G
        Vec3f(+size, +size, -size),  // H
    };

    mesh.indices = { A, B, B, C, C, D, D, A, E, F, F, G, G, H, H, E, A, E, B, F, D, H, C, G };

    return mesh;
}

}  // namespace cave::math
