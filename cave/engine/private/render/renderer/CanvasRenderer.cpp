#include "CanvasRenderer.h"

#include "engine/private/renderer/gpu_resource.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
// @TODO: move IRenderDevice to render?
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/render/rhi/PipelineState.h"

namespace cave::render {

using namespace ::cave::math;

namespace {

struct PrimBatch {
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
    const GpuTexture* tex{};
};

struct BuildResult {
    Ref<GpuMesh> mesh;
    Vector<PrimBatch> batches;
};

BuildResult BuildCanvasMesh(IRenderDevice& device,
                            CanvasBucket&& bucket) {
    BuildResult result;

    const uint32_t item_count = static_cast<uint32_t>(bucket.shapes.size());
    if (item_count == 0) {
        return result;
    }

    std::sort(bucket.shapes.begin(), bucket.shapes.end(),
              [](const PrimShape& a, const PrimShape& b) {
                  return a.tex < b.tex;
              });

    Vector<uint32_t> indices;
    Vector<Vec3f> positions;
    Vector<Vec2f> uvs;
    Vector<Vec4f> colors;

    indices.reserve(item_count * 6);
    positions.reserve(item_count * 4);
    uvs.reserve(item_count * 4);
    colors.reserve(item_count * 4);

    auto begin_batch = [&](const GpuTexture* tex) {
        PrimBatch batch;
        batch.tex = tex;
        batch.index_offset = static_cast<uint32_t>(indices.size());
        batch.index_count = 0;
        result.batches.push_back(batch);
    };

    GpuTexture* current_tex = nullptr;

    for (const PrimShape item : bucket.shapes) {
        if (result.batches.empty() || item.tex != current_tex) {
            current_tex = item.tex;
            begin_batch(current_tex);
        }

        PrimBatch& batch = result.batches.back();

        const uint32_t offset = static_cast<uint32_t>(positions.size());
        const uint32_t before_index_count = static_cast<uint32_t>(indices.size());

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

                indices.push_back(offset + 0);
                indices.push_back(offset + 1);
                indices.push_back(offset + 3);

                indices.push_back(offset + 0);
                indices.push_back(offset + 3);
                indices.push_back(offset + 2);
            } break;

            default: {
                LOG_WARN(LogChannel::Render,
                         "primitive {} not supported",
                         std::to_underlying(item.type));
            } break;
        }

        const uint32_t after_index_count = static_cast<uint32_t>(indices.size());
        batch.index_count += after_index_count - before_index_count;
    }

    // Remove empty batches caused by unsupported shapes.
    std::erase_if(result.batches, [](const PrimBatch& batch) {
        return batch.index_count == 0;
    });

    const uint32_t vertex_count = static_cast<uint32_t>(positions.size());
    const uint32_t index_count = static_cast<uint32_t>(indices.size());

    if (vertex_count == 0 || index_count == 0 || result.batches.empty()) {
        return {};
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

    auto mesh_result = device.CreateMeshImpl(desc, buffer_descs, &index_desc);
    if (!mesh_result) {
        return {};
    }

    result.mesh = *mesh_result;
    return result;
}

}  // namespace

CanvasRenderer::CanvasRenderer(AssetRegistry& asset_registry)
    : m_asset_reg(asset_registry) {
}

void CanvasRenderer::ensureDefaultTexture() {
    if (m_default_texture) {
        return;
    }

    if (auto handle_opt = m_asset_reg.findByPath<ImageAsset>("@persist://textures/white@1x1")) {
        if (const ImageAsset* image = handle_opt.unwrap_unchecked().get()) {
            m_default_texture = image->gpu_texture;
        }
    }
}

void CanvasRenderer::drawCanvas(IRenderDevice& device,
                                ICanvas& canvas,
                                ViewId view_id) {
    ensureDefaultTexture();

    DEV_ASSERT(m_default_texture);

    CanvasBucket bucket;
    if (!canvas.takeBucket(view_id, bucket)) {
        return;
    }

    auto result = BuildCanvasMesh(device, std::move(bucket));
    if (!result.mesh) {
        return;
    }

    device.SetMesh(result.mesh.get());
    // @TODO: clean this up
    auto pso = m_screen_space ? PSO_UI_OVERLAY : PSO_PRIMITIVE;
    device.SetPipelineState(pso);

    constexpr int kSpriteSlot = 0;
    for (const PrimBatch& batch : result.batches) {
        const uint64_t tex = (batch.tex ? batch.tex : m_default_texture.get())->GetHandle();
        device.BindTexture(Dimension::TEXTURE_2D, tex, kSpriteSlot);
        device.DrawElements(batch.index_count, batch.index_offset);
    }
    device.UnbindTexture(Dimension::TEXTURE_2D, kSpriteSlot);
}

}  // namespace cave::render
