#include "graphics_manager.h"

#include "engine/private/assets/image_asset.h"
#include "engine/private/core/base/random.h"
#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/core/math/frustum.h"
#include "engine/private/core/math/geometry.h"
#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/render/render_graph/CommonPasses.h"
#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/render/render_graph/RenderGraphPredefined.h"
#include "engine/private/renderer/frame_data.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/RenderSystem.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {
#include "shader_resource_defines.hlsl.h"
}  // namespace cave

// @TODO: refactor
#include "engine/private/renderer/path_tracer/path_tracer.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef GetMessage
#undef GetMessage
#endif

namespace cave {

using namespace math;
using namespace render;

template<typename T>
static auto CreateUniformCheckSize(GraphicsManager& p_graphics_manager, uint32_t p_max_count) {
    static_assert(sizeof(T) % 256 == 0);
    GpuBufferDesc buffer_desc{};
    buffer_desc.slot = T::GetUniformBufferSlot();
    buffer_desc.element_count = p_max_count;
    buffer_desc.element_size = sizeof(T);
    return p_graphics_manager.CreateConstantBuffer(buffer_desc);
}

auto GraphicsManager::InitializeImpl() -> Result<void> {
    m_enableValidationLayer = DVAR_GET_BOOL(gfx_gpu_validation);

    const int num_frames = (GetBackend() == Backend::D3D12) ? NUM_FRAMES_IN_FLIGHT : 1;
    m_frameContexts.resize(num_frames);
    for (int i = 0; i < num_frames; ++i) {
        m_frameContexts[i] = CreateFrameContext();
    }
    if (auto res = InitializeInternal(); !res) {
        return CAVE_ERROR(res.error());
    }

    if (m_backend == Backend::METAL) {
        return Result<void>();
    }

    const Vector2i frame_size = DVAR_GET_IVEC2(resolution);
    RenderGraphBuilderConfig config;
    config.frameWidth = frame_size.x;
    config.frameHeight = frame_size.y;
    if (auto res = RenderGraphBuilderExt::Create3D(config); !res) {
        return CAVE_ERROR(res.error());
    } else {
        m_render_graph = *res;
    }

    for (int i = 0; i < num_frames; ++i) {
        FrameContext& frame_context = *m_frameContexts[i].get();
        frame_context.batchCb = *::cave::CreateUniformCheckSize<PerBatchConstantBuffer>(*this, 4096 * 16);
        frame_context.passCb = *::cave::CreateUniformCheckSize<PerPassConstantBuffer>(*this, 32);
        frame_context.materialCb = *::cave::CreateUniformCheckSize<MaterialConstantBuffer>(*this, 2048 * 16);
        frame_context.boneCb = *::cave::CreateUniformCheckSize<BoneConstantBuffer>(*this, 16);
        frame_context.emitterCb = *::cave::CreateUniformCheckSize<EmitterConstantBuffer>(*this, 32);
        frame_context.pointShadowCb = *::cave::CreateUniformCheckSize<PointShadowConstantBuffer>(*this, 6 * MAX_POINT_LIGHT_SHADOW_COUNT);
        frame_context.perFrameCb = *::cave::CreateUniformCheckSize<PerFrameConstantBuffer>(*this, 1);
    }

    DEV_ASSERT(m_pipelineStateManager);

    if (auto res = m_pipelineStateManager->Initialize(); !res) {
        return CAVE_ERROR(res.error());
    }

    // create meshes
    // @TODO: refactor
    m_skyboxBuffers = *CreateMesh(MakeSkyBoxMesh());
    m_boxBuffers = *CreateMesh(MakeBoxMesh());

    m_initialized = true;
    return Result<void>();
}

void GraphicsManager::EventReceived(std::shared_ptr<IEvent> p_event) {
    if (ResizeEvent* e = dynamic_cast<ResizeEvent*>(p_event.get()); e) {
        OnWindowResize(e->GetWidth(), e->GetHeight());
    }
}

void GraphicsManager::SetPipelineState(PipelineStateName p_name) {
    SetPipelineStateImpl(p_name);
}

void GraphicsManager::RequestTexture(ImageAsset* p_image) {
    m_loadedImages.push(p_image);
}

void GraphicsManager::RequestMesh(MeshAsset* p_mesh) {
    m_loadedMeshes.push(p_mesh);
}

void GraphicsManager::UpdateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) {
    unused(p_desc);
    unused(p_buffer);
    CRASH_NOW();
}

auto GraphicsManager::CreateMesh(const MeshAsset& p_mesh) -> Result<std::shared_ptr<GpuMesh>> {
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
        buffer_desc.type = GpuBufferType::VERTEX;
        buffer_desc.element_count = in.elementCount;
        buffer_desc.element_size = in.strideInByte;
        buffer_desc.initial_data = data[index];
        buffer_desc.dynamic = is_dynamic;
    }

    GpuBufferDesc ib_desc;
    GpuBufferDesc* ib_desc_ptr = nullptr;
    if (!p_mesh.indices.empty()) {
        ib_desc = GpuBufferDesc{
            .type = GpuBufferType::INDEX,
            .element_size = sizeof(uint32_t),
            .element_count = (uint32_t)p_mesh.indices.size(),
            .initial_data = p_mesh.indices.data(),
        };
        ib_desc_ptr = &ib_desc;
    }

    auto ret = CreateMeshImpl(desc, count, vb_descs.data(), ib_desc_ptr);
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

std::shared_ptr<GpuTexture> GraphicsManager::CreateTexture(ImageAsset* p_image) {
    DEV_ASSERT(p_image);

    GpuTextureDesc texture_desc{};
    SamplerDesc sampler_desc{};
    FillTextureAndSamplerDesc(p_image, texture_desc, sampler_desc);

    p_image->gpu_texture = CreateTexture(texture_desc, sampler_desc);
    return p_image->gpu_texture;
}

void GraphicsManager::Update() {
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

        auto views = m_app->GetRenderSystem()->GetFrameData();

        for (const FrameData& data : views) {

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

            // @HACK
            switch (m_backend) {
                case Backend::VULKAN:
                case Backend::METAL:
                    break;
                default: {
                    m_render_graph->Execute(data, *this);
                } break;
            }
        }

        // @TODO: remove this
        // if (p_scene) {
        //    UpdateEmitters(*p_scene);
        //}

        Render();
        EndFrame();
        Present();
        MoveToNextFrame();
    }
}

void GraphicsManager::UpdateBufferData(const GpuBufferDesc& p_desc, const GpuStructuredBuffer* p_buffer) {
    unused(p_desc);
    unused(p_buffer);
}

void GraphicsManager::BeginFrame() {
}

void GraphicsManager::EndFrame() {
}

void GraphicsManager::MoveToNextFrame() {
}

std::shared_ptr<FrameContext> GraphicsManager::CreateFrameContext() {
    return std::make_unique<FrameContext>();
}

void GraphicsManager::BeginDrawPass(const Framebuffer* p_framebuffer) {
    for (auto& texture : p_framebuffer->outSrvs) {
        if (texture->slot >= 0) {
            UnbindTexture(texture->desc.dimension, texture->slot);
        }
    }
}

void GraphicsManager::EndDrawPass(const Framebuffer* p_framebuffer) {
    UnsetRenderTarget();
    for (auto& texture : p_framebuffer->outSrvs) {
        if (texture->slot >= 0) {
            BindTexture(texture->desc.dimension, texture->GetHandle(), texture->slot);
        }
    }
}

std::shared_ptr<GpuTexture> GraphicsManager::CreateTexture(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) {
    auto texture = CreateTextureImpl(p_texture_desc, p_sampler_desc);
    if (p_texture_desc.type != AttachmentType::NONE) {
        auto [_, inserted] = m_resourceLookup.try_emplace(texture->desc.name, texture);
        if (!inserted) {
            CRASH_NOW();
        }
        m_resourceLookup[p_texture_desc.name] = texture;
    }
    return texture;
}

std::shared_ptr<GpuTexture> GraphicsManager::FindTexture(std::string_view p_name) const {
    if (m_resourceLookup.empty()) {
        return nullptr;
    }

    auto it = m_resourceLookup.find(p_name);
    if (it == m_resourceLookup.end()) {
        return nullptr;
    }
    return it->second;
}

uint64_t GraphicsManager::GetFinalImage() const {
    const GpuTexture* texture = FindTexture(RG_RES_POST_PROCESS).get();

    if (texture) {
        return texture->GetHandle();
    }

    return 0;
}

void GraphicsManager::UpdateEmitters(const Scene& p_scene) {
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

void GraphicsManager::DrawSkybox() {
    SetMesh(m_skyboxBuffers.get());
    DrawElements(m_skyboxBuffers->desc.drawCount);
}

}  // namespace cave
