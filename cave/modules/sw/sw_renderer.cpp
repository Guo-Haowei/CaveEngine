#include "sw_renderer.h"

#include "engine/assets/mesh_asset.h"
#include "engine/math/box.h"
#include "engine/systems/job_system/job_system.h"

namespace cave::rs {

// config
static constexpr int TILE_SIZE = 128;

// barycentric
// P = uA + vB + wC (where w = 1 - u - v)
// P = uA + vB + C - uC - vC => (C - P) + u(A - C) + v(B - C) = 0
// uCA + vCB + PC = 0
// [u v 1] [CAx CBx PCx] = 0
// [u v 1] [CAy CBy PCy] = 0
// [u v 1] is the cross product

/**
 * Perspective correct linear interpolation
 * https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
 */
static void NdcToViewport(Vector4f& p_position) {
    DEV_ASSERT(p_position.w != 0.0f);
    float inv_w = 1.0f / p_position.w;
    p_position.x *= inv_w;
    p_position.y *= inv_w;
    p_position.z *= inv_w;
    p_position.w = inv_w;

    p_position.x = 0.5f + 0.5f * p_position.x;
    p_position.y = 0.5f + 0.5f * p_position.y;
}

OutTriangle SwGraphicsManager::ProcessTriangle(const VSInput& vs_in0,
                                               const VSInput& vs_in1,
                                               const VSInput& vs_in2) {
    SwPipeline* pipeline = m_state.pipeline;
    VSOutput vs_out0 = pipeline->ProcessVertex(vs_in0);
    VSOutput vs_out1 = pipeline->ProcessVertex(vs_in1);
    VSOutput vs_out2 = pipeline->ProcessVertex(vs_in2);

    // TODO: front plane clipping
    constexpr float t = 1.0f;
    if (vs_out0.position.z / vs_out0.position.w > t ||
        vs_out1.position.z / vs_out1.position.w > t ||
        vs_out2.position.z / vs_out2.position.w > t) {
        OutTriangle triangle;
        triangle.discarded = true;
        return triangle;
    }

    NdcToViewport(vs_out0.position);
    NdcToViewport(vs_out1.position);
    NdcToViewport(vs_out2.position);

    // face culling
    Vector3f ab3d(vs_out0.position.x - vs_out1.position.x, vs_out0.position.y - vs_out1.position.y, vs_out0.position.z - vs_out1.position.z);
    Vector3f ac3d(vs_out0.position.x - vs_out2.position.x, vs_out0.position.y - vs_out2.position.y, vs_out0.position.z - vs_out2.position.z);
    Vector3f normal = cross(ab3d, ac3d);

    if (normal.z < 0.0f) {  // Cull backface
        OutTriangle triangle;
        triangle.discarded = true;
        return triangle;
    }

    return OutTriangle{ vs_out0, vs_out1, vs_out2 };
}

static int TileNumber(int p_tile_size, int p_length) {
    int rem = (p_length % p_tile_size) != 0;
    return (p_length / p_tile_size) + rem;
}

void SwGraphicsManager::ProcessFragment(OutTriangle& vs_out, int tx, int ty) {
    RenderTarget* rt = m_state.rt;
    SwPipeline* pipeline = m_state.pipeline;

    const int width = rt->m_depthBuffer.m_width;
    const int height = rt->m_depthBuffer.m_height;

    const VSOutput& vs_out0 = vs_out.p0;
    const VSOutput& vs_out1 = vs_out.p1;
    const VSOutput& vs_out2 = vs_out.p2;
    const Vector2f a(vs_out0.position.x * width, vs_out0.position.y * height);
    const Vector2f b(vs_out1.position.x * width, vs_out1.position.y * height);
    const Vector2f c(vs_out2.position.x * width, vs_out2.position.y * height);

    // discard if A, B and C are on the same line
    const Vector2f BA = a - b;
    const Vector2f CA = a - c;
    const Vector2f CB = b - c;
    if (BA.x * CA.y == BA.y * CA.x) {
        return;
    }

    auto& colorBuffer = rt->m_colorBuffer;
    DepthBuffer& depthBuffer = rt->m_depthBuffer;
    const uint32_t varyingFlags = pipeline->GetVaryingFlags();

    const Vector2f _min(tx * TILE_SIZE, ty * TILE_SIZE);
    const Vector2f _max(
        glm::min(width - 1, (tx + 1) * TILE_SIZE),
        glm::min(height - 1, (ty + 1) * TILE_SIZE));
    const Rect screenBox(_min, _max);
    Rect triangleBox{};
    triangleBox.ExpandPoint(a);
    triangleBox.ExpandPoint(b);
    triangleBox.ExpandPoint(c);
    triangleBox.UnionBox(screenBox);
    bool intersect = triangleBox.IsValid();
    // discard if not intersect
    if (!intersect) {
        return;
    }

    // rasterization
    Vector2f min = triangleBox.GetMin();
    Vector2f max = triangleBox.GetMax();
    for (int _y = (int)min.y; _y < (int)max.y; ++_y) {
        for (int _x = (int)min.x; _x < (int)max.x; ++_x) {
            Vector3f a_(CA.x, CB.x, c.x - _x);
            Vector3f b_(CA.y, CB.y, c.y - _y);
            Vector3f uvw = cross(a_, b_);
            if (uvw.z == 0.0f) {
                continue;
            }
            uvw /= uvw.z;
            uvw.z -= (uvw.x + uvw.y);
            Vector3f bCoord = uvw;

            static const float epsilon = glm::epsilon<float>();
            const float sum = bCoord.x + bCoord.y + bCoord.z;
            bool test = (bCoord.x >= 0.0f && bCoord.y >= 0.0f && bCoord.z >= 0.0f && std::abs(sum - 1.0f) <= epsilon);
            if (test == true) {
                // depth test
                VSOutput output;
                output.position = bCoord.x * vs_out0.position + bCoord.y * vs_out1.position + bCoord.z * vs_out2.position;
                output.position.z = 0.5f * output.position.z + 0.5f;  // normalize to [0, 1]
                const size_t index = (height - _y - 1) * width + _x;

                float depth = output.position.z;

                if (depthBuffer.m_buffer[index] < depth) {
                    continue;
                }

                depthBuffer.m_buffer[index] = depth;
                // corrected barycentric coordinates
                float x = bCoord.x * vs_out0.position.w;
                float y = bCoord.y * vs_out1.position.w;
                float z = bCoord.z * vs_out2.position.w;
                float xyzSum = x + y + z;
                bCoord.x = x / xyzSum;
                bCoord.y = y / xyzSum;
                bCoord.z = z / xyzSum;

                if (varyingFlags & VARYING_NORMAL) {
                    output.normal = bCoord.x * vs_out0.normal + bCoord.y * vs_out1.normal + bCoord.z * vs_out2.normal;
                    output.normal = normalize(output.normal);
                }
                if (varyingFlags & VARYING_COLOR) {
                    output.color = bCoord.x * vs_out0.color + bCoord.y * vs_out1.color + bCoord.z * vs_out2.color;
                }
                if (varyingFlags & VARYING_UV) {
                    // NOTE: slow here! probably due to unaligned simd operation
                    output.uv = bCoord.x * vs_out0.uv + bCoord.y * vs_out1.uv + bCoord.z * vs_out2.uv;
                }
                if (varyingFlags & VARYING_WORLD_POSITION) {
                    output.world_position = bCoord.x * vs_out0.world_position + bCoord.y * vs_out1.world_position + bCoord.z * vs_out2.world_position;
                }

                // fragment shader
                colorBuffer.m_buffer[index] = Vector4f(pipeline->ProcessFragment(output), 1.0f);
            }
        }
    }
}

void SwGraphicsManager::DrawArrayInternal(std::vector<OutTriangle>& trigs) {
    // remove invalid triangles
    trigs.erase(remove_if(trigs.begin(),
                          trigs.end(),
                          [](OutTriangle& trig) { return trig.discarded; }),
                trigs.end());

    RenderTarget* rt = m_state.rt;
    const int width = rt->m_depthBuffer.m_width;
    const int height = rt->m_depthBuffer.m_height;

    const int col = TileNumber(TILE_SIZE, width);
    const int row = TileNumber(TILE_SIZE, height);

    jobsystem::Context ctx;
    ctx.Dispatch(row, 1, [&](jobsystem::JobArgs args) {
        for (int c = 0; c < col; ++c) {
            for (OutTriangle& triangle : trigs) {
                ProcessFragment(triangle, c, args.jobIndex);
            }
        }
    });

    ctx.Wait();
}

void SwGraphicsManager::SetPipelineStateImpl(PipelineStateName p_name) {
    unused(p_name);
}

void SwGraphicsManager::Clear(const Framebuffer* p_framebuffer,
                              ClearFlags p_flags,
                              const float* p_clear_color,
                              float p_clear_depth,
                              uint8_t,
                              int) {
    unused(p_framebuffer);

    if (p_flags & ClearFlags::CLEAR_COLOR_BIT) {
        auto clear_color = reinterpret_cast<const Vector4f*>(p_clear_color);
        m_state.rt->m_colorBuffer.clear(*clear_color);
    }
    if (p_flags & ClearFlags::CLEAR_DEPTH_BIT) {
        m_state.rt->m_depthBuffer.clear(p_clear_depth);
    }
}

void SwGraphicsManager::DrawElements(uint32_t p_count, uint32_t p_offset) {
    DEV_ASSERT(p_count % 3 == 0);
    DEV_ASSERT(p_offset % 3 == 0);

    const int triangle_count = static_cast<int>(p_count) / 3;
    std::vector<OutTriangle> triangles(triangle_count);

    jobsystem::Context ctx;
    ctx.Dispatch(triangle_count, 8, [&](jobsystem::JobArgs args) {
        const uint32_t idx = args.jobIndex;
        const VSInput* vertices = m_state.vertices;
        const uint32_t* indices = m_state.indices;
        const VSInput& p0 = vertices[indices[idx * 3 + 0]];
        const VSInput& p1 = vertices[indices[idx * 3 + 1]];
        const VSInput& p2 = vertices[indices[idx * 3 + 2]];
        triangles[idx] = ProcessTriangle(p0, p1, p2);
    });
    ctx.Wait();

    DrawArrayInternal(triangles);
}

void SwGraphicsManager::DrawArrays(uint32_t p_count, uint32_t p_offsest) {
    DEV_ASSERT(p_count % 3 == 0);
    DEV_ASSERT(p_offsest % 3 == 0);

    const int triangle_count = static_cast<int>(p_count) / 3;
    std::vector<OutTriangle> triangles(triangle_count);

    jobsystem::Context ctx;
    ctx.Dispatch(triangle_count, 8, [&](jobsystem::JobArgs args) {
        const uint32_t idx = args.jobIndex;
        const VSInput* vertices = m_state.vertices;
        const VSInput& p0 = vertices[idx * 3 + 0];
        const VSInput& p1 = vertices[idx * 3 + 1];
        const VSInput& p2 = vertices[idx * 3 + 2];
        triangles[idx] = ProcessTriangle(p0, p1, p2);
    });
    ctx.Wait();

    DrawArrayInternal(triangles);
}

auto SwGraphicsManager::CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> {
    GpuMeshDesc desc;
    desc.drawCount = static_cast<uint32_t>(p_mesh.indices.size());
    auto mesh = std::make_shared<SwMesh>(desc);
    unused(p_mesh);
    mesh->indices = p_mesh.indices;
    mesh->vertices.resize(p_mesh.positions.size());

    for (size_t i = 0; i < mesh->vertices.size(); ++i) {
        mesh->vertices[i].position = Vector4f(p_mesh.positions[i], 1.0f);
        mesh->vertices[i].normal = Vector4f(p_mesh.normals[i], 0.0f);
        mesh->vertices[i].uv = p_mesh.texcoords_0[i];
    }

    return mesh;
}

void SwGraphicsManager::SetMesh(const GpuMesh* p_mesh) {
    const SwMesh* mesh = static_cast<const SwMesh*>(p_mesh);
    m_state.vertices = mesh->vertices.data();
    m_state.indices = mesh->indices.data();
}

}  // namespace cave::rs
