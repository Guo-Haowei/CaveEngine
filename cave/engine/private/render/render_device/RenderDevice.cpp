#include "RenderDevice.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/render/rhi/Sampler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/renderer/RenderSubmission.h"
#include "engine/private/render/render_graph/CompiledGraph.h"

// @TODO: determine if includes are necessary
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/core/math/geometry.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/render/renderer/GraphicsDvars.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {
#include "shader_resource_defines.slang.h"
}  // namespace cave

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace cave::render {

using namespace math;

template<typename T>
static auto CreateUniformCheckSize(RenderDevice& device, uint32_t max_count) {
    static_assert(sizeof(T) % 256 == 0);

    auto buffer_desc = GpuResourceDesc::constantBuffer(sizeof(T), max_count);
    buffer_desc.deprecated_slot = T::GetUniformBufferSlot();
    return device.createConstantBuffer(buffer_desc);
}

auto RenderDevice::InitializeImpl() -> Result<void> {
    m_enable_validation_layer = DVAR_GET_BOOL(gfx_gpu_validation);

    const int num_frames = (m_app->backend() == Backend::D3d12) ? kNumFramesInFlight : 1;
    m_frame_contexts.resize(num_frames);
    for (int i = 0; i < num_frames; ++i) {
        m_frame_contexts[i] = createFrameContext();
    }
    if (auto res = InitializeInternal(); !res) {
        return CAVE_ERROR(res.error());
    }

    for (int i = 0; i < num_frames; ++i) {
        FrameContext& frame_context = *m_frame_contexts[i].get();
        frame_context.batch_cb = *CreateUniformCheckSize<PerBatchConstantBuffer>(*this, 4096 * 16);
        frame_context.view_cb = *CreateUniformCheckSize<PerViewConstantBuffer>(*this, 32);
        frame_context.material_cb = *CreateUniformCheckSize<MaterialConstantBuffer>(*this, 2048 * 16);
        frame_context.pass_cb = *CreateUniformCheckSize<PerPassConstantBuffer>(*this, 128);
        frame_context.bone_cb = *CreateUniformCheckSize<BoneConstantBuffer>(*this, 16);
        frame_context.per_frame_cb = *CreateUniformCheckSize<PerFrameConstantBuffer>(*this, 1);
        frame_context.emitter_cb = *CreateUniformCheckSize<EmitterConstantBuffer>(*this, 32);
        frame_context.point_shadow_cb = *CreateUniformCheckSize<PointShadowConstantBuffer>(*this, 6 * MAX_POINT_LIGHT_SHADOW_COUNT);
    }

    DEV_ASSERT(m_pipeline_state_manager);

    if (auto res = m_pipeline_state_manager->initialize(m_capabilities); !res) {
        return CAVE_ERROR(res.error());
    }

    // create meshes
    // @TODO: refactor
    m_skybox_buffers = *createMesh(MakeSkyBoxMesh());

    m_initialized = true;
    return Result<void>();
}

void RenderDevice::eventReceived(std::shared_ptr<IEvent> p_event) {
    if (ResizeEvent* e = dynamic_cast<ResizeEvent*>(p_event.get()); e) {
        onWindowResize(e->GetWidth(), e->GetHeight());
    }
}

void RenderDevice::requestTexture(ImageAsset* image) {
    m_loaded_images.push(image);
}

void RenderDevice::requestMesh(MeshAsset* mesh) {
    m_loaded_meshes.push(mesh);
}

void RenderDevice::updateBuffer(const GpuResourceDesc&, GpuResource*) {
    CRASH_NOW();
}

auto RenderDevice::createMesh(const MeshAsset& mesh) -> Result<Ref<GpuMesh>> {
    constexpr uint32_t count = std::to_underlying(VertexAttributeName::COUNT);
    std::array<VertexAttributeName, count> attribs = {
        VertexAttributeName::POSITION,
        VertexAttributeName::NORMAL,
        VertexAttributeName::TEXCOORD_0,
        VertexAttributeName::TANGENT,
        VertexAttributeName::JOINTS_0,
        VertexAttributeName::WEIGHTS_0,
        VertexAttributeName::COLOR_0,
        VertexAttributeName::TEXCOORD_1,
    };

    std::array<const void*, count> data = {
        mesh.positions.data(),
        mesh.normals.data(),
        mesh.texcoords_0.data(),
        mesh.tangents.data(),
        mesh.joints_0.data(),
        mesh.weights_0.data(),
        mesh.color_0.data(),
        mesh.texcoords_1.data(),
    };

    std::array<GpuResourceDesc, count> vb_descs;

    GpuMeshDesc desc;
    desc.enabled_vertex_count = count;
    desc.draw_count = static_cast<uint32_t>(mesh.indices.empty() ? mesh.positions.size() : mesh.indices.size());

    for (int index = 0; index < (int)attribs.size(); ++index) {
        const auto& in = mesh.attributes[std::to_underlying(attribs[index])];
        auto& layout = desc.vertex_layout[index];
        layout.slot = index;
        layout.offset_in_byte = in.offsetInByte;
        layout.stride_in_byte = in.strideInByte;

        auto& buffer_desc = vb_descs[index];
        buffer_desc = GpuResourceDesc::vertexBuffer(in.strideInByte, in.elementCount, data[index]);
    }

    GpuResourceDesc ib_desc{};
    GpuResourceDesc* ib_desc_ptr = nullptr;
    if (!mesh.indices.empty()) {
        ib_desc = GpuResourceDesc::indexBuffer(sizeof(uint32_t), (uint32_t)mesh.indices.size(), mesh.indices.data());
        ib_desc_ptr = &ib_desc;
    }

    auto gpu_mesh = createMeshImpl(desc, vb_descs, ib_desc_ptr);
    if (!gpu_mesh) {
        return CAVE_ERROR(gpu_mesh.error());
    }

    mesh.gpuResource = *gpu_mesh;
    return gpu_mesh;
}

// @TODO: refactor this
static void FillTextureAndSamplerDesc(const ImageAsset* image,
                                      GpuResourceDesc& texture_desc,
                                      SamplerDesc& sampler_desc) {
    DEV_ASSERT(image);
    bool is_hdr_file = false;

    switch (image->format) {
        case PixelFormat::R32_FLOAT:
        case PixelFormat::R32G32_FLOAT:
        case PixelFormat::R32G32B32_FLOAT:
        case PixelFormat::R32G32B32A32_FLOAT: {
            is_hdr_file = true;
        } break;
        default: {
        } break;
    }

    texture_desc.format = image->format;
    texture_desc.dimension = Dimension::Texture2D;
    texture_desc.width = image->width;
    texture_desc.height = image->height;
    texture_desc.array_size = 1;
    texture_desc.bind_flags |= BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
    texture_desc.initial_data = image->buffer.data();
    texture_desc.mip_levels = 1;
    texture_desc.misc_flags |= RESOURCE_MISC_GENERATE_MIPS;

    if (is_hdr_file) {
        sampler_desc.min_filter = MinFilter::Linear;
        sampler_desc.mag_filter = MagFilter::Linear;
        sampler_desc.address_u = sampler_desc.address_v = AddressMode::Clamp;
        // p_texture_desc.bindFlags &= (~BIND_RENDER_TARGET);
    } else {
        sampler_desc.min_filter = MinFilter::LinearMipmapLinear;
        sampler_desc.mag_filter = MagFilter::Linear;
    }

    // override sampler
    if (image->sampler == ImageAsset::Sampler::Point) {
        sampler_desc.min_filter = MinFilter::Point;
        sampler_desc.mag_filter = MagFilter::Point;
    }

    switch (image->address_mode) {
        case ImageAsset::AddressMode::Wrap: {
            sampler_desc.address_u = AddressMode::Wrap;
            sampler_desc.address_v = AddressMode::Wrap;
            sampler_desc.address_w = AddressMode::Wrap;
        } break;
        default:
            break;
    }
}

Ref<GpuResource> RenderDevice::createTexture(ImageAsset* image) {
    DEV_ASSERT(image);

    GpuResourceDesc texture_desc{};
    SamplerDesc sampler_desc{};
    FillTextureAndSamplerDesc(image, texture_desc, sampler_desc);

    image->gpu_texture = createTexture(texture_desc, sampler_desc);
    return image->gpu_texture;
}

void RenderDevice::submit(Owner<render::RenderSubmission>&& submission) {
    CAVE_PROFILE_EVENT();

    // @TODO: make it a function
    auto loaded_images = m_loaded_images.pop_all();
    while (!loaded_images.empty()) {
        ImageAsset* image = loaded_images.front();
        DEV_ASSERT(image);
        loaded_images.pop();

        if (!image->gpu_texture) {
            createTexture(image);
        }
    }
    auto loaded_meshes = m_loaded_meshes.pop_all();
    while (!loaded_meshes.empty()) {
        MeshAsset* mesh = loaded_meshes.front();
        DEV_ASSERT(mesh);
        loaded_meshes.pop();

        if (!mesh->gpuResource) {
            auto res = createMesh(*mesh);
            DEV_ASSERT(res);
        }
    }

    // @TODO: support multiple views
    {
        CAVE_PROFILE_EVENT("Render");
        beginFrame();

        int idx = 0;
        for (const FrameData& data : submission->frame_data) {

            auto& frame = getCurrentFrame();
            updateConstantBuffer(frame.batch_cb.get(), data.batchCache.buffer);
            updateConstantBuffer(frame.material_cb.get(), data.materialCache.buffer);
            updateConstantBuffer(frame.bone_cb.get(), data.boneCache.buffer);
            updateConstantBuffer(frame.view_cb.get(), data.viewCache);
            updateConstantBuffer(frame.pass_cb.get(), data.passCache);

            updateConstantBuffer<PointShadowConstantBuffer, 6 * MAX_POINT_LIGHT_SHADOW_COUNT>(
                frame.point_shadow_cb.get(),
                data.pointShadowCache);
            updateConstantBuffer(frame.per_frame_cb.get(),
                                 &data.perFrameCache,
                                 sizeof(PerFrameConstantBuffer));

            getCommandList()->bindConstantBufferSlot<PerFrameConstantBuffer>(frame.per_frame_cb.get(), 0);

            const auto& graph = submission->render_graph[idx++];
            for (const CompiledPass& pass : graph->getCompiledPass()) {
                execute(data, pass);
            }
        }

        render();
        endFrame();
        present();
        moveToNextFrame();
    }
}

void RenderDevice::updateBufferData(const GpuResourceDesc&, const GpuResource*) {
    CRASH_NOW();
}

void RenderDevice::beginFrame() {
}

void RenderDevice::endFrame() {
}

void RenderDevice::moveToNextFrame() {
}

Ref<FrameContext> RenderDevice::createFrameContext() {
    return MakeRef<FrameContext>();
}

Ref<GpuResource> RenderDevice::createTexture(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) {
    auto texture = createTextureImpl(texture_desc, sampler_desc);
    return texture;
}

void RenderDevice::updateEmitters(const Scene& p_scene) {
    unused(p_scene);
#if 0
    for (auto [id, emitter] : p_scene.m_ParticleEmitterComponents) {
        if (!emitter.particleBuffer) {
            // create buffer
            emitter.counterBuffer = *CreateStructuredBuffer({
                .elementSize = sizeof(ParticleCounter),
                .elementCount = 1,
            });
            emitter.deadBuffer = *CreateStructuredBuffer({
                .elementSize = sizeof(int),
                .elementCount = MAX_PARTICLE_COUNT,
            });
            emitter.aliveBuffer[0] = *CreateStructuredBuffer({
                .elementSize = sizeof(int),
                .elementCount = MAX_PARTICLE_COUNT,
            });
            emitter.aliveBuffer[1] = *CreateStructuredBuffer({
                .elementSize = sizeof(int),
                .elementCount = MAX_PARTICLE_COUNT,
            });
            emitter.particleBuffer = *CreateStructuredBuffer({
                .elementSize = sizeof(Particle),
                .elementCount = MAX_PARTICLE_COUNT,
            });

            SetPipelineState(PSO_PARTICLE_INIT);

            BindStructuredBuffer(GetGlobalParticleCounterSlot(), emitter.counterBuffer.get());
            BindStructuredBuffer(GetGlobalDeadIndicesSlot(), emitter.deadBuffer.get());
            Dispatch(MAX_PARTICLE_COUNT / PARTICLE_LOCAL_SIZE, 1, 1);
            UnbindStructuredBuffer(GetGlobalParticleCounterSlot());
            UnbindStructuredBuffer(GetGlobalDeadIndicesSlot());
        }
    }
#endif
}

void RenderDevice::drawSkybox() {
    auto& cmd = *getCommandList();

    cmd.setMesh(m_skybox_buffers.get());
    cmd.drawIndexed(m_skybox_buffers->desc.draw_count);
}

void RenderDevice::beginPass(const CompiledPass& pass) {
    RenderingInfo desc{
        .colors = pass.colors,
        .depth = pass.depth,
    };

    // @TODO: build render target
    uint32_t width = 0, height = 0;
    bool has_target = false;
    if (!desc.colors.empty()) {
        width = desc.colors[0].tex->desc.width;
        height = desc.colors[0].tex->desc.height;
        has_target = true;
    }
    if (desc.depth) {
        width = desc.depth->tex->desc.width;
        height = desc.depth->tex->desc.height;
        has_target = true;
    }

    ICommandList* cmd = getCommandList();
    DEV_ASSERT(cmd);

    if (has_target) {
        cmd->beginRendering(desc);
        cmd->setViewport(pass.viewport ? *pass.viewport : Viewport(width, height));
    }

    // bind srvs
    for (uint32_t i = 0; i < pass.srvs.size(); ++i) {
        if (const GpuResource* resource = pass.srvs[i].get()) {
            DEV_ASSERT(resource->desc.bind_flags & BIND_SHADER_RESOURCE);
            cmd->bindTexture(resource->desc.dimension, resource->getSrvHandle(), i);
        }
    }
    // bind uavs
    for (uint32_t i = 0; i < pass.uavs.size(); ++i) {
        if (GpuResource* uav = pass.uavs[i].get()) {
            DEV_ASSERT(uav->desc.bind_flags & BIND_UNORDERED_ACCESS);
            cmd->bindUnorderedAccessView(i, uav);
        }
    }

    // transition srvs
#if 0
    for (auto& texture : pass.srvs) {
        if (!texture) continue;
        auto* d3d_tex = static_cast<D3d12GpuTexture*>(texture.get());
        transitionTexture(d3d_tex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
#endif
}

void RenderDevice::endPass(const CompiledPass& pass) {
    ICommandList* cmd = getCommandList();
    DEV_ASSERT(cmd);

    // unbind srvs
    for (uint32_t i = 0; i < pass.srvs.size(); ++i) {
        if (const GpuResource* srv = pass.srvs[i].get()) {
            DEV_ASSERT(srv->desc.bind_flags & BIND_SHADER_RESOURCE);
            cmd->unbindTexture(srv->desc.dimension, i);
        }
    }

    // unbind uavs
    for (uint32_t i = 0; i < pass.uavs.size(); ++i) {
        if (GpuResource* uav = pass.uavs[i].get()) {
            DEV_ASSERT(uav->desc.bind_flags & BIND_UNORDERED_ACCESS);
            cmd->unbindUnorderedAccessView(i);
        }
    }

#if 0
    for (auto& uav : pass.uavs) {
        if (!uav) continue;
        auto* d3d_tex = static_cast<D3d12GpuTexture*>(uav.get());

        auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(d3d_tex->texture.Get());
        m_graphics_command_list->ResourceBarrier(1, &barrier);
    }
#endif

    cmd->endRendering();
}

void RenderDevice::execute(const FrameData& framedata, const CompiledPass& pass) {
    ICommandList* cmd = this->getCommandList();
    DEV_ASSERT(cmd);
    RenderPassExcutionContext ctx = {
        .frameData = framedata,
        .pass = pass,
        .command = *cmd,
        .device = *this,
        .services = m_app->services(),
    };

    cmd->pushDebugGroup(pass.name);
    beginPass(pass);

    if (pass.execute_func) {
        auto& device = ctx.device;

        device.getCommandList()->bindConstantBufferSlot<PerPassConstantBuffer>(device.getCurrentFrame().pass_cb.get(), pass.pass_id);

        pass.execute_func(ctx);
    }

    endPass(pass);

    cmd->popDebugGroup();
}

}  // namespace cave::render
