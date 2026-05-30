#include "RenderDevice.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/renderer/RenderSubmission.h"
#include "engine/private/render/render_graph/CompiledGraph.h"

// @TODO: determine if includes are necessary
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/core/math/geometry.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {
#include "shader_resource_defines.hlsl.h"
}  // namespace cave

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef GetMessage
#undef GetMessage
#endif

namespace cave::render {

using namespace math;

template<typename T>
static auto CreateUniformCheckSize(RenderDevice& p_graphics_manager, uint32_t p_max_count) {
    static_assert(sizeof(T) % 256 == 0);
    GpuBufferDesc buffer_desc{};
    buffer_desc.slot = T::GetUniformBufferSlot();
    buffer_desc.element_count = p_max_count;
    buffer_desc.element_size = sizeof(T);
    return p_graphics_manager.CreateConstantBuffer(buffer_desc);
}

auto RenderDevice::InitializeImpl() -> Result<void> {
    m_enableValidationLayer = DVAR_GET_BOOL(gfx_gpu_validation);

    const int num_frames = (m_app->GetBackend() == Backend::Direct3D12) ? NUM_FRAMES_IN_FLIGHT : 1;
    m_frameContexts.resize(num_frames);
    for (int i = 0; i < num_frames; ++i) {
        m_frameContexts[i] = CreateFrameContext();
    }
    if (auto res = InitializeInternal(); !res) {
        return CAVE_ERROR(res.error());
    }

    for (int i = 0; i < num_frames; ++i) {
        FrameContext& frame_context = *m_frameContexts[i].get();
        frame_context.batchCb = *CreateUniformCheckSize<PerBatchConstantBuffer>(*this, 4096 * 16);
        frame_context.passCb = *CreateUniformCheckSize<PerPassConstantBuffer>(*this, 32);
        frame_context.materialCb = *CreateUniformCheckSize<MaterialConstantBuffer>(*this, 2048 * 16);
        frame_context.boneCb = *CreateUniformCheckSize<BoneConstantBuffer>(*this, 16);
        frame_context.emitterCb = *CreateUniformCheckSize<EmitterConstantBuffer>(*this, 32);
        frame_context.pointShadowCb = *CreateUniformCheckSize<PointShadowConstantBuffer>(*this, 6 * MAX_POINT_LIGHT_SHADOW_COUNT);
        frame_context.perFrameCb = *CreateUniformCheckSize<PerFrameConstantBuffer>(*this, 1);
    }

    DEV_ASSERT(m_pipelineStateManager);

    if (auto res = m_pipelineStateManager->Initialize(); !res) {
        return CAVE_ERROR(res.error());
    }

    // create meshes
    // @TODO: refactor
    m_skybox_buffers = *CreateMesh(MakeSkyBoxMesh());

    m_initialized = true;
    return Result<void>();
}

void RenderDevice::EventReceived(std::shared_ptr<IEvent> p_event) {
    if (ResizeEvent* e = dynamic_cast<ResizeEvent*>(p_event.get()); e) {
        OnWindowResize(e->GetWidth(), e->GetHeight());
    }
}

void RenderDevice::SetPipelineState(PipelineStateName p_name) {
    SetPipelineStateImpl(p_name);
}

void RenderDevice::RequestTexture(ImageAsset* p_image) {
    m_loadedImages.push(p_image);
}

void RenderDevice::RequestMesh(MeshAsset* p_mesh) {
    m_loadedMeshes.push(p_mesh);
}

void RenderDevice::UpdateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) {
    unused(p_desc);
    unused(p_buffer);
    CRASH_NOW();
}

auto RenderDevice::CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> {
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
        p_mesh.positions.data(),
        p_mesh.normals.data(),
        p_mesh.texcoords_0.data(),
        p_mesh.tangents.data(),
        p_mesh.joints_0.data(),
        p_mesh.weights_0.data(),
        p_mesh.color_0.data(),
        p_mesh.texcoords_1.data(),
    };

    std::array<GpuBufferDesc, count> vb_descs;

    const bool is_dynamic = false;

    GpuMeshDesc desc;
    desc.enabledVertexCount = count;
    desc.drawCount = static_cast<uint32_t>(p_mesh.indices.empty() ? p_mesh.positions.size() : p_mesh.indices.size());

    for (int index = 0; index < (int)attribs.size(); ++index) {
        const auto& in = p_mesh.attributes[std::to_underlying(attribs[index])];
        auto& layout = desc.vertexLayout[index];
        layout.slot = index;
        layout.offsetInByte = in.offsetInByte;
        layout.strideInByte = in.strideInByte;

        auto& buffer_desc = vb_descs[index];
        buffer_desc.slot = index;
        buffer_desc.type = GpuBufferType::Vertex;
        buffer_desc.element_count = in.elementCount;
        buffer_desc.element_size = in.strideInByte;
        buffer_desc.initial_data = data[index];
        buffer_desc.dynamic = is_dynamic;
    }

    GpuBufferDesc ib_desc;
    GpuBufferDesc* ib_desc_ptr = nullptr;
    if (!p_mesh.indices.empty()) {
        ib_desc = GpuBufferDesc{
            .type = GpuBufferType::Index,
            .element_size = sizeof(uint32_t),
            .element_count = (uint32_t)p_mesh.indices.size(),
            .initial_data = p_mesh.indices.data(),
        };
        ib_desc_ptr = &ib_desc;
    }

    auto ret = CreateMeshImpl(desc, vb_descs, ib_desc_ptr);
    if (!ret) {
        return CAVE_ERROR(ret.error());
    }

    p_mesh.gpuResource = *ret;
    return ret;
}

// @TODO: refactor this
static void FillTextureAndSamplerDesc(const ImageAsset* p_image, GpuTextureDesc& p_texture_desc, SamplerDesc& p_sampler_desc) {
    DEV_ASSERT(p_image);
    bool is_hdr_file = false;

    switch (p_image->format) {
        case PixelFormat::R32_FLOAT:
        case PixelFormat::R32G32_FLOAT:
        case PixelFormat::R32G32B32_FLOAT:
        case PixelFormat::R32G32B32A32_FLOAT: {
            is_hdr_file = true;
        } break;
        default: {
        } break;
    }

    p_texture_desc.format = p_image->format;
    p_texture_desc.dimension = Dimension::TEXTURE_2D;
    p_texture_desc.width = p_image->width;
    p_texture_desc.height = p_image->height;
    p_texture_desc.arraySize = 1;
    p_texture_desc.bindFlags |= BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
    p_texture_desc.initialData = p_image->buffer.data();
    p_texture_desc.mipLevels = 1;
    p_texture_desc.miscFlags |= RESOURCE_MISC_GENERATE_MIPS;

    if (is_hdr_file) {
        p_sampler_desc.minFilter = MinFilter::LINEAR;
        p_sampler_desc.magFilter = MagFilter::LINEAR;
        p_sampler_desc.addressU = p_sampler_desc.addressV = AddressMode::CLAMP;
        // p_texture_desc.bindFlags &= (~BIND_RENDER_TARGET);
    } else {
        p_sampler_desc.minFilter = MinFilter::LINEAR_MIPMAP_LINEAR;
        p_sampler_desc.magFilter = MagFilter::LINEAR;
    }

    // override sampler
    if (p_image->sampler == ImageAsset::Sampler::Point) {
        p_sampler_desc.minFilter = MinFilter::POINT;
        p_sampler_desc.magFilter = MagFilter::POINT;
    }
}

std::shared_ptr<GpuTexture> RenderDevice::CreateTexture(ImageAsset* p_image) {
    DEV_ASSERT(p_image);

    GpuTextureDesc texture_desc{};
    SamplerDesc sampler_desc{};
    FillTextureAndSamplerDesc(p_image, texture_desc, sampler_desc);

    p_image->gpu_texture = CreateTexture(texture_desc, sampler_desc);
    return p_image->gpu_texture;
}

void RenderDevice::Submit(std::unique_ptr<render::RenderSubmission>&& p_submission) {
    CAVE_PROFILE_EVENT();

    // @TODO: make it a function
    auto loaded_images = m_loadedImages.pop_all();
    while (!loaded_images.empty()) {
        ImageAsset* image = loaded_images.front();
        DEV_ASSERT(image);
        loaded_images.pop();

        if (!image->gpu_texture) {
            CreateTexture(image);
        }
    }
    auto loaded_meshes = m_loadedMeshes.pop_all();
    while (!loaded_meshes.empty()) {
        MeshAsset* mesh = loaded_meshes.front();
        DEV_ASSERT(mesh);
        loaded_meshes.pop();

        if (!mesh->gpuResource) {
            CreateMesh(*mesh);
        }
    }

    // @TODO: support multiple views
    {
        CAVE_PROFILE_EVENT("Render");
        BeginFrame();

        int idx = 0;
        for (const FrameData& data : p_submission->frame_data) {

            auto& frame = GetCurrentFrame();
            UpdateConstantBuffer(frame.batchCb.get(), data.batchCache.buffer);
            UpdateConstantBuffer(frame.materialCb.get(), data.materialCache.buffer);
            UpdateConstantBuffer(frame.boneCb.get(), data.boneCache.buffer);
            UpdateConstantBuffer(frame.passCb.get(), data.passCache);
            // UpdateConstantBuffer(frame.emitterCb.get(), data->emitterCache);

            UpdateConstantBuffer<PointShadowConstantBuffer, 6 * MAX_POINT_LIGHT_SHADOW_COUNT>(
                frame.pointShadowCb.get(),
                data.pointShadowCache);
            UpdateConstantBuffer(frame.perFrameCb.get(),
                                 &data.perFrameCache,
                                 sizeof(PerFrameConstantBuffer));

            BindConstantBufferSlot<PerFrameConstantBuffer>(frame.perFrameCb.get(), 0);

            auto& graph = p_submission->render_graph[idx++];
            for (const CompiledPass& pass : graph->GetCompiledPass()) {
                Execute(data, pass);
            }
        }

        Render();
        EndFrame();
        Present();
        MoveToNextFrame();
    }
}

void RenderDevice::UpdateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) {
    unused(p_desc);
    unused(p_buffer);
}

void RenderDevice::BeginFrame() {
}

void RenderDevice::EndFrame() {
}

void RenderDevice::MoveToNextFrame() {
}

std::shared_ptr<FrameContext> RenderDevice::CreateFrameContext() {
    return std::make_unique<FrameContext>();
}

std::shared_ptr<GpuTexture> RenderDevice::CreateTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) {
    auto texture = CreateTextureImpl(p_texture_desc, p_sampler_desc);
    return texture;
}

void RenderDevice::UpdateEmitters(const Scene& p_scene) {
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

void RenderDevice::DrawSkybox() {
    SetMesh(m_skybox_buffers.get());
    DrawElements(m_skybox_buffers->desc.drawCount);
}

void RenderDevice::BeginPass(const CompiledPass& p_pass) {
    // bind srvs
    for (int i = 0; i < (int)p_pass.srvs.size(); ++i) {
        if (const GpuTexture* srv = p_pass.srvs[i].get()) {
            DEV_ASSERT(srv->desc.bindFlags & BIND_SHADER_RESOURCE);
            BindTexture(srv->desc.dimension, srv->GetHandle(), i);
        }
    }
    // bind uavs
    for (int i = 0; i < (int)p_pass.uavs.size(); ++i) {
        if (GpuTexture* uav = p_pass.uavs[i].get()) {
            DEV_ASSERT(uav->desc.bindFlags & BIND_UNORDERED_ACCESS);
            BindUnorderedAccessView(i, uav);
        }
    }

    RenderTargetDesc desc{
        .colors = p_pass.colors,
        .depth = p_pass.depth,
    };

    // @TODO: build render target
    uint32_t width = 0, height = 0;
    bool has_rt_or_ds = false;
    if (!desc.colors.empty()) {
        width = desc.colors[0].tex->desc.width;
        height = desc.colors[0].tex->desc.height;
        has_rt_or_ds = true;
    }
    if (desc.depth) {
        width = desc.depth->tex->desc.width;
        height = desc.depth->tex->desc.height;
        has_rt_or_ds = true;
    }

    if (has_rt_or_ds) {
        SetRenderTargets(desc);
        SetViewport(p_pass.viewport ? *p_pass.viewport : Viewport(width, height));
    }
}

void RenderDevice::EndPass(const CompiledPass& p_pass) {
    UnsetRenderTargets();

    // unbind srvs
    for (int i = 0; i < (int)p_pass.srvs.size(); ++i) {
        if (const GpuTexture* srv = p_pass.srvs[i].get()) {
            DEV_ASSERT(srv->desc.bindFlags & BIND_SHADER_RESOURCE);
            UnbindTexture(srv->desc.dimension, i);
        }
    }

    // unbind uavs
    for (int i = 0; i < (int)p_pass.uavs.size(); ++i) {
        if (GpuTexture* uav = p_pass.uavs[i].get()) {
            DEV_ASSERT(uav->desc.bindFlags & BIND_UNORDERED_ACCESS);
            BindUnorderedAccessView(i, uav);
            UnbindUnorderedAccessView(i);
        }
    }
}

void RenderDevice::Execute(const FrameData& p_data, const CompiledPass& p_pass) {
    RenderPassExcutionContext ctx{
        .frameData = p_data,
        .pass = p_pass,
        .cmd = *this,
    };

    BeginEvent(p_pass.name);
    BeginPass(p_pass);
    p_pass.func(ctx);
    EndPass(p_pass);
    EndEvent();
}

}  // namespace cave::render
