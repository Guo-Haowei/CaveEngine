#include "CanvasRenderer.h"

#include "engine/private/renderer/gpu_resource.h"

// @TODO: move IRenderDevice to render
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/render/rhi/PipelineState.h"

namespace cave::render {

using namespace ::cave::math;

namespace {

Ref<GpuMesh> BuildCanvasMesh(IRenderDevice& device,
                             const CanvasBucket& bucket) {
    const uint32_t item_count = static_cast<uint32_t>(bucket.shapes.size());
    if (item_count == 0) {
        return nullptr;
    }

    Vector<uint32_t> indices;
    Vector<Vec3f> positions;
    Vector<Vec2f> uvs;
    Vector<Vec4f> colors;

    indices.reserve(item_count * 6);
    positions.reserve(item_count * 4);
    uvs.reserve(item_count * 4);
    colors.reserve(item_count * 4);

    for (const auto& item : bucket.shapes) {
        const uint32_t offset = static_cast<uint32_t>(positions.size());
        switch (item.type) {
            case PrimShapeType::Rect: {
                positions.push_back(item.vertices[0].pos);
                positions.push_back(item.vertices[1].pos);
                positions.push_back(item.vertices[2].pos);
                positions.push_back(item.vertices[3].pos);
                uvs.push_back(item.vertices[0].uv);
                uvs.push_back(item.vertices[1].uv);
                uvs.push_back(item.vertices[2].uv);
                uvs.push_back(item.vertices[3].uv);
                colors.push_back(item.vertices[0].color);
                colors.push_back(item.vertices[1].color);
                colors.push_back(item.vertices[2].color);
                colors.push_back(item.vertices[3].color);

                indices.push_back(0 + offset);
                indices.push_back(1 + offset);
                indices.push_back(2 + offset);

                indices.push_back(0 + offset);
                indices.push_back(3 + offset);
                indices.push_back(2 + offset);

            } break;
            default: {
                LOG_WARN(LogChannel::Render, "primitive {} not supported", std::to_underlying(item.type));
            } break;
        }
    }

    const uint32_t vertex_count = static_cast<uint32_t>(positions.size());
    const uint32_t index_count = static_cast<uint32_t>(indices.size());

    if (index_count == 0) {
        return nullptr;
    }

    std::array<GpuBufferDesc, 3> buffer_descs;
    buffer_descs[0] = {
        .type = GpuBufferType::Vertex,
        .slot = 0,
        .element_size = sizeof(Vec3f),
        .element_count = vertex_count,
        .initial_data = positions.data(),
    };
    buffer_descs[1] = {
        .type = GpuBufferType::Vertex,
        .slot = 1,
        .element_size = sizeof(Vec2f),
        .element_count = vertex_count,
        .initial_data = uvs.data(),
    };
    buffer_descs[2] = {
        .type = GpuBufferType::Vertex,
        .slot = 2,
        .element_size = sizeof(Vec4f),
        .element_count = vertex_count,
        .initial_data = colors.data(),
    };

    GpuBufferDesc index_desc = {
        .type = GpuBufferType::Index,
        .element_size = sizeof(uint32_t),
        .element_count = index_count,
        .initial_data = indices.data(),
    };

    GpuMeshDesc desc;
    desc.drawCount = index_count;
    desc.enabledVertexCount = 3;
    desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vec3f), 0 };
    desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vec2f), 0 };
    desc.vertexLayout[2] = GpuMeshDesc::VertexLayout{ 2, sizeof(Vec4f), 0 };

    return *(device.CreateMeshImpl(desc, buffer_descs, &index_desc));
}

}  // namespace

void CanvasRenderer::drawCanvas(IRenderDevice& device,
                                ICanvas& canvas,
                                ViewId view_id) {
    auto primitives = canvas.primitives();
    for (const auto& bucket : primitives) {
        if (bucket.view_id == view_id) {
            auto mesh = BuildCanvasMesh(device, bucket);
            if (mesh) {
                device.SetMesh(mesh.get());
                device.SetPipelineState(PSO_PRIMITIVE);
                device.DrawElements(mesh->desc.drawCount);
            }
            break;
        }
    }
}

}  // namespace cave::render
