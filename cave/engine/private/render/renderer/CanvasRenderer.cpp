#include "CanvasRenderer.h"

#include "engine/private/render/rhi/gpu_resource.h"

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
                            CanvasBucket&& bucket,
                            const GpuTexture* default_tex) {
    BuildResult result;

    const uint32_t item_count = static_cast<uint32_t>(bucket.shapes.size());
    if (item_count == 0) {
        return result;
    }

    Vector<uint32_t> indices;
    Vector<Vec3f> positions;
    Vector<Vec2f> uvs;
    Vector<Vec4f> colors;
    Vector<Vec2u> tex_ids;

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

    const GpuTexture* current_tex = nullptr;

    for (const PrimShape& item : bucket.shapes) {
        if (result.batches.empty() || item.tex != current_tex) {
            current_tex = item.tex;
            begin_batch(current_tex);
        }

        PrimBatch& batch = result.batches.back();

        const uint32_t offset = static_cast<uint32_t>(positions.size());
        const uint32_t before_index_count = static_cast<uint32_t>(indices.size());

        const uint64_t tex_id = (current_tex ? current_tex : default_tex)->GetResidentHandle();
        const Vec2u packed_tex_id{ 0xFFFFFFFF & tex_id, tex_id << 32 };

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

                tex_ids.push_back(packed_tex_id);
                tex_ids.push_back(packed_tex_id);
                tex_ids.push_back(packed_tex_id);
                tex_ids.push_back(packed_tex_id);

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

    std::array<GpuBufferDesc, 4> buffer_descs;
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
    buffer_descs[3] = {
        .type = GpuBufferType::Vertex,
        .slot = 3,
        .element_size = sizeof(Vec2u),
        .element_count = vertex_count,
        .initial_data = tex_ids.data(),
    };

    GpuBufferDesc index_desc = {
        .type = GpuBufferType::Index,
        .element_size = sizeof(uint32_t),
        .element_count = index_count,
        .initial_data = indices.data(),
    };

    GpuMeshDesc desc;
    desc.draw_count = index_count;
    desc.enabled_vertex_count = static_cast<uint32_t>(buffer_descs.size());
    desc.vertex_layout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vec3f), 0 };
    desc.vertex_layout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Vec2f), 0 };
    desc.vertex_layout[2] = GpuMeshDesc::VertexLayout{ 2, sizeof(Vec4f), 0 };
    desc.vertex_layout[3] = GpuMeshDesc::VertexLayout{ 3, sizeof(Vec2u), 0 };

    auto mesh_result = device.createMeshImpl(desc, buffer_descs, &index_desc);
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

    auto result = BuildCanvasMesh(device, std::move(bucket), m_default_texture.get());
    if (!result.mesh) {
        return;
    }

    ICommandList* command = device.getCommandList();

    command->setMesh(result.mesh.get());
    // @TODO: clean this up
    const PipelineStateName pso = m_screen_space ? PSO_UI_OVERLAY : PSO_PRIMITIVE;
    command->setPipelineState(pso);

    constexpr int kSpriteSlot = 0;
    for (const PrimBatch& batch : result.batches) {
        const uint64_t tex = (batch.tex ? batch.tex : m_default_texture.get())->GetHandle();
        command->bindTexture(Dimension::Texture2D, tex, kSpriteSlot);
        command->drawIndexed(batch.index_count, batch.index_offset);
    }
    command->unbindTexture(Dimension::Texture2D, kSpriteSlot);
}

}  // namespace cave::render
