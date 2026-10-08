#include "MetalRenderDevice.h"

#include "MetalHelpers.h"
#include "MetalPipelineStateManager.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include <imgui/backends/imgui_impl_metal.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <algorithm>
#include <cstring>

namespace cave::render {

namespace {

struct MetalBuffer final : GpuBuffer {
    using GpuBuffer::GpuBuffer;
    void* object{};
    ~MetalBuffer() override {
        if (object)
            CFRelease(object);
    }
    uint64_t GetHandle() const final { return reinterpret_cast<uint64_t>(object); }
};

struct MetalTexture final : GpuTexture {
    using GpuTexture::GpuTexture;
    void* object{};
    ~MetalTexture() override {
        if (object)
            CFRelease(object);
    }
    uint64_t GetResidentHandle() const final { return 0; }
    uint64_t GetHandle() const final { return reinterpret_cast<uint64_t>(object); }
    uint64_t GetUavHandle() const final { return GetHandle(); }
};

struct MetalConstantBuffer final : GpuConstantBuffer {
    using GpuConstantBuffer::GpuConstantBuffer;
    void* object{};
    ~MetalConstantBuffer() override {
        if (object)
            CFRelease(object);
    }
};

id<MTLSamplerState> createSampler(id<MTLDevice> device, MTLSamplerDescriptor* desc) {
    id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:desc];
    return sampler;
}

} //  namespace

MetalRenderDevice::MetalRenderDevice() : RenderDevice("MetalRenderDevice", rhi::Backend::Metal, 1) { m_pipeline_state_manager = MakeOwner<MetalPipelineStateManager>(); }

MetalRenderDevice::~MetalRenderDevice() = default;

auto MetalRenderDevice::InitializeInternal() -> Result<void> {
    @autoreleasepool {
        auto* display = dynamic_cast<GlfwDisplayService*>(&m_app->services().displayService());
        if (!display)
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal backend requires GLFW Cocoa display");
        m_window = display->GetGlfwWindow();
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device)
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "MTLCreateSystemDefaultDevice failed");
        id<MTLCommandQueue> queue = [device newCommandQueue];
        if (!queue)
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Could not create Metal command queue");
        m_device = (__bridge_retained void*)device;
        m_command_queue = (__bridge_retained void*)queue;
        MTLSamplerDescriptor* sampler_desc = [MTLSamplerDescriptor new];
        sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
        sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
        sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
        sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
        id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:sampler_desc];
        m_default_sampler = (__bridge_retained void*)sampler;
        NSWindow* window = glfwGetCocoaWindow(m_window);
        NSView* view = window.contentView;
        CAMetalLayer* layer = [CAMetalLayer layer];
        layer.device = device;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        layer.framebufferOnly = YES;
        view.wantsLayer = YES;
        view.layer = layer;
        m_layer = (__bridge_retained void*)layer;

        if (m_app->specification().enableImgui) {
            if (auto* imgui = m_app->services().imgui) {
                auto initialize_cb = [this]() {
                    ImGui_ImplMetal_Init((__bridge id<MTLDevice>)m_device);
                    ImGui_ImplMetal_CreateDeviceObjects((__bridge id<MTLDevice>)m_device);
                };
                auto finalize_cb = []() { ImGui_ImplMetal_Shutdown(); };
                imgui->setRenderCallbacks(std::move(initialize_cb), std::move(finalize_cb));
            }
        }

#define SAMPLER_STATE(REG, NAME, DESC)                                                       \
    if (auto sampler = createSampler(device, FillMetalSamplerDesc(DESC)); !sampler) {        \
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to create sampler {}", #NAME); \
    } else {                                                                                 \
        m_samplers[REG] = (__bridge_retained void*)sampler;                                  \
    }
#include "sampler.slang.h"
#undef SAMPLER_STATE
    }
    return Result<void>();
}

void MetalRenderDevice::FinalizeImpl() {
    m_pipeline_state_manager->finalize();
    for (void** p : { &m_encoder, &m_drawable, &m_command_buffer, &m_layer, &m_default_sampler, &m_command_queue, &m_device }) {
        if (*p) {
            CFRelease(*p);
            *p = nullptr;
        }
    }
}

void MetalRenderDevice::beginFrame() {
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_command_queue;
    m_command_buffer = (__bridge_retained void*)[queue commandBuffer];
}

void MetalRenderDevice::endFrame() { unsetRenderTargets(); }

void MetalRenderDevice::setRenderTargets(const RenderTargetDesc& target) {
    unsetRenderTargets();
    MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
    for (NSUInteger i = 0; i < target.colors.size() && i < 8; ++i) {
        const auto& color = target.colors[i];
        auto* tex = reinterpret_cast<const MetalTexture*>(color.tex.get());
        pass.colorAttachments[i].texture = (__bridge id<MTLTexture>)tex->object;
        pass.colorAttachments[i].loadAction = color.load == LoadOp::Clear ? MTLLoadActionClear : color.load == LoadOp::Load ? MTLLoadActionLoad : MTLLoadActionDontCare;
        pass.colorAttachments[i].storeAction = MTLStoreActionStore;
        pass.colorAttachments[i].clearColor = MTLClearColorMake(color.clear_color[0], color.clear_color[1], color.clear_color[2], color.clear_color[3]);
    }

    if (target.depth) {
        const auto* tex = reinterpret_cast<const MetalTexture*>(target.depth->tex.get());
        DEV_ASSERT(tex && tex->object);
        pass.depthAttachment.texture = (__bridge id<MTLTexture>)tex->object;
        pass.depthAttachment.loadAction = target.depth->depth_load == LoadOp::Clear ? MTLLoadActionClear : MTLLoadActionLoad;
        pass.depthAttachment.storeAction = MTLStoreActionStore;
        pass.depthAttachment.clearDepth = target.depth->clear_depth;
        if (target.depth->tex->desc.format == PixelFormat::D24_UNORM_S8_UINT) {
            pass.stencilAttachment.texture = (__bridge id<MTLTexture>)tex->object;
            pass.stencilAttachment.loadAction = target.depth->stencil_load == LoadOp::Clear ? MTLLoadActionClear : MTLLoadActionLoad;
            pass.stencilAttachment.storeAction = MTLStoreActionStore;
            pass.stencilAttachment.clearStencil = target.depth->clear_stencil;
        }
    }

    id<MTLCommandBuffer> cb = (__bridge id<MTLCommandBuffer>)m_command_buffer;
    m_encoder = (__bridge_retained void*)[cb renderCommandEncoderWithDescriptor:pass];

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_encoder;
    {
#define SAMPLER_STATE(REG, NAME, DESC) [encoder setFragmentSamplerState:(__bridge id<MTLSamplerState>)m_samplers[REG] atIndex:REG];
#include "sampler.slang.h"
#undef SAMPLER_STATE
    }
    {
        for (uint32_t slot = 0; slot < m_constant_buffers.size(); ++slot) {
            if (void* buffer = m_constant_buffers[slot]) {
                id<MTLBuffer> metalBuffer = (__bridge id<MTLBuffer>)buffer;

                [encoder setVertexBuffer:metalBuffer offset:0 atIndex:slot];
                [encoder setFragmentBuffer:metalBuffer offset:0 atIndex:slot];
            }
        }
    }
}

void MetalRenderDevice::unsetRenderTargets() {
    if (!m_encoder)
        return;
    [(id<MTLRenderCommandEncoder>)CFBridgingRelease(m_encoder) endEncoding];
    m_encoder = nullptr;
}

void MetalRenderDevice::clear(const RenderTargetDesc&) {
    // Render graph attachment load actions perform clears when an encoder begins.
}

void MetalRenderDevice::setViewport(const Viewport& v) {
    if (!m_encoder)
        return;
    [(id<MTLRenderCommandEncoder>)m_encoder setViewport:MTLViewport{ (double)v.topLeftX, (double)v.topLeftY, (double)v.width, (double)v.height, 0.0, 1.0 }];
}

void MetalRenderDevice::setStencilRef(uint32_t ref) {
    m_stencil_ref = ref;
    if (m_encoder)
        [(id<MTLRenderCommandEncoder>)m_encoder setStencilReferenceValue:ref];
}

void MetalRenderDevice::setBlendState(const BlendDesc&, const float*, uint32_t) {
    // Blend is immutable in Metal; put it in the pipeline descriptor once the
    // engine's blend descriptor mapping is finalized.
}

auto MetalRenderDevice::createBuffer(const GpuBufferDesc& d) -> Result<Ref<GpuBuffer>> {
    const size_t bytes = size_t(d.element_count) * d.element_size;
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    id<MTLBuffer> b = [device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
    if (!b)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal buffer allocation failed ({} bytes)", bytes);
    if (d.initial_data && bytes)
        memcpy(b.contents, d.initial_data, bytes);
    auto result = MakeRef<MetalBuffer>(d);
    result->object = (__bridge_retained void*)b;
    return result;
}

void MetalRenderDevice::updateBuffer(const GpuBufferDesc& d, GpuBuffer* base) {
    auto* b = reinterpret_cast<MetalBuffer*>(base);
    id<MTLBuffer> mb = (__bridge id<MTLBuffer>)b->object;
    const size_t bytes = size_t(d.element_count) * d.element_size;
    if (bytes <= mb.length && d.initial_data)
        memcpy((uint8_t*)mb.contents, d.initial_data, bytes);
}

auto MetalRenderDevice::createMeshImpl(const GpuMeshDesc& d, std::span<const GpuBufferDesc> vbs, const GpuBufferDesc* ib) -> Result<Ref<GpuMesh>> {
    auto mesh = MakeRef<MetalGpuMesh>(d);
    for (uint32_t i = 0; i < vbs.size(); ++i) {
        if (!vbs[i].element_count)
            continue;
        auto b = createBuffer(vbs[i]);
        if (!b)
            return CAVE_ERROR(b.error());
        mesh->vertexBuffers[i] = *b;
    }
    if (ib) {
        auto b = createBuffer(*ib);
        if (!b)
            return CAVE_ERROR(b.error());
        mesh->indexBuffer = *b;
    }
    return mesh;
}

void MetalRenderDevice::setMesh(const GpuMesh* mesh) {
    m_current_mesh = const_cast<GpuMesh*>(mesh);
    if (!m_encoder)
        return;
    auto* e = (id<MTLRenderCommandEncoder>)m_encoder;
    if (!mesh) {
        for (NSUInteger i = 0; i < 31; ++i)
            [e setVertexBuffer:nil offset:0 atIndex:i];
        return;
    }
    for (uint32_t i = 0; i < mesh->vertexBuffers.size(); ++i)
        if (mesh->vertexBuffers[i]) {
            auto* b = reinterpret_cast<const MetalBuffer*>(mesh->vertexBuffers[i].get());
            [e setVertexBuffer:(__bridge id<MTLBuffer>)b->object offset:0 atIndex:i];
        }
}

void MetalRenderDevice::setPipelineStateImpl(PipelineStateName name) {
    auto* p = reinterpret_cast<MetalPipelineState*>(m_pipeline_state_manager->findPSO(name));
    m_pipeline = p ? p->state : nullptr;
    m_current_pso = p;
    if (m_encoder && m_pipeline && p->desc.type == PipelineStateType::GRAPHICS) {
        auto* encoder = (id<MTLRenderCommandEncoder>)m_encoder;
        [encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)m_pipeline];

        if (p->depth_state)
            [encoder setDepthStencilState:(__bridge id<MTLDepthStencilState>)p->depth_state];
        [encoder setStencilReferenceValue:m_stencil_ref];
        if (p->desc.rasterizer_desc) {
            MTLCullMode cull = MTLCullModeNone;
            switch (p->desc.rasterizer_desc->cullMode) {
            case CullMode::FRONT:
                cull = MTLCullModeFront;
                break;
            case CullMode::BACK:
                cull = MTLCullModeBack;
                break;
            default:
                break;
            }
            [encoder setCullMode:cull];
            [encoder setFrontFacingWinding:p->desc.rasterizer_desc->frontCounterClockwise ? MTLWindingCounterClockwise : MTLWindingClockwise];
        }
    }
}

void MetalRenderDevice::drawElements(uint32_t count, uint32_t offset) {
    if (!m_encoder || !m_current_mesh)
        return;
    auto* mesh = static_cast<MetalGpuMesh*>(m_current_mesh);
    auto* index = reinterpret_cast<MetalBuffer*>(mesh->indexBuffer.get());
    if (!index)
        return;
    [(id<MTLRenderCommandEncoder>)m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:(__bridge id<MTLBuffer>)index->object indexBufferOffset:offset * sizeof(uint32_t)];
}

void MetalRenderDevice::drawElementsInstanced(uint32_t n, uint32_t count, uint32_t offset) {
    if (!m_encoder || !m_current_mesh)
        return;
    auto* mesh = static_cast<MetalGpuMesh*>(m_current_mesh);
    auto* index = reinterpret_cast<MetalBuffer*>(mesh->indexBuffer.get());
    if (!index)
        return;
    [(id<MTLRenderCommandEncoder>)m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:(__bridge id<MTLBuffer>)index->object indexBufferOffset:offset * sizeof(uint32_t) instanceCount:n];
}

void MetalRenderDevice::drawArrays(uint32_t count, uint32_t offset) {
    if (m_encoder)
        [(id<MTLRenderCommandEncoder>)m_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:offset vertexCount:count];
}

void MetalRenderDevice::drawArraysInstanced(uint32_t n, uint32_t count, uint32_t offset) {
    if (m_encoder)
        [(id<MTLRenderCommandEncoder>)m_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:offset vertexCount:count instanceCount:n];
}

auto MetalRenderDevice::createConstantBuffer(const GpuBufferDesc& d) -> Result<Ref<GpuConstantBuffer>> {
    const size_t bytes = size_t(d.element_count) * d.element_size;
    id<MTLBuffer> b = [(__bridge id<MTLDevice>)m_device newBufferWithLength:bytes options:MTLResourceStorageModeShared];
    if (!b)
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal constant buffer allocation failed");
    auto result = MakeRef<MetalConstantBuffer>(d);
    result->object = (__bridge_retained void*)b;
    DEV_ASSERT(d.slot == m_constant_buffers.size());
    m_constant_buffers.push_back(result->object);
    return result;
}

void MetalRenderDevice::updateConstantBuffer(const GpuConstantBuffer* base, const void* data, size_t size) {
    auto* b = reinterpret_cast<const MetalConstantBuffer*>(base);
    id<MTLBuffer> mb = (__bridge id<MTLBuffer>)b->object;
    if (data && size <= mb.length)
        memcpy(mb.contents, data, size);
}

void MetalRenderDevice::bindConstantBufferRange(const GpuConstantBuffer* base, uint32_t size, uint32_t offset) {
    auto* b = reinterpret_cast<const MetalConstantBuffer*>(base);
    id<MTLBuffer> mb = (__bridge id<MTLBuffer>)b->object;
    if (m_encoder) {
        [(id<MTLRenderCommandEncoder>)m_encoder setVertexBuffer:mb offset:offset atIndex:base->GetSlot()];
        [(id<MTLRenderCommandEncoder>)m_encoder setFragmentBuffer:mb offset:offset atIndex:base->GetSlot()];
    }
    (void)size;
}

Ref<GpuTexture> MetalRenderDevice::createTextureImpl(const GpuTextureDesc& d, const SamplerDesc&) {
    if (d.dimension != Dimension::Texture2D)
        return nullptr;
    const MTLPixelFormat format = ToMetalTextureFormat(d.format);
    if (format == MTLPixelFormatInvalid)
        return nullptr;
    MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format width:d.width height:d.height mipmapped:d.mipLevels > 1];
    td.usage = MTLTextureUsageShaderRead;
    if ((d.bindFlags & BIND_RENDER_TARGET) || (d.bindFlags & BIND_DEPTH_STENCIL))
        td.usage |= MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    if (d.bindFlags & BIND_UNORDERED_ACCESS)
        td.usage |= MTLTextureUsageShaderWrite;
    td.storageMode = MTLStorageModeShared;
    id<MTLTexture> t = [(__bridge id<MTLDevice>)m_device newTextureWithDescriptor:td];
    if (!t)
        return nullptr;
    if (d.initialData && format != MTLPixelFormatDepth32Float && format != MTLPixelFormatDepth24Unorm_Stencil8) {
        const NSUInteger bytes_per_pixel = format == MTLPixelFormatRGBA8Unorm ? 4 : format == MTLPixelFormatR32Float ? 4 : format == MTLPixelFormatRG32Float ? 8 : 16;
        [t replaceRegion:MTLRegionMake2D(0, 0, d.width, d.height) mipmapLevel:0 withBytes:d.initialData bytesPerRow:d.width * bytes_per_pixel];
    }
    auto result = MakeRef<MetalTexture>(d);
    result->object = (__bridge_retained void*)t;
    return result;
}

void MetalRenderDevice::bindTexture(Dimension, uint64_t h, int slot) {
    if (slot < 0 || slot >= (int)m_bound_textures.size())
        return;
    m_bound_textures[slot] = h;
    if (!m_encoder || !h)
        return;
    id<MTLTexture> t = (__bridge id<MTLTexture>)reinterpret_cast<void*>(h);
    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentTexture:t atIndex:slot];
    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentSamplerState:(__bridge id<MTLSamplerState>)m_default_sampler atIndex:slot];
}

void MetalRenderDevice::unbindTexture(Dimension, int slot) {
    if (slot < 0 || slot >= (int)m_bound_textures.size())
        return;
    m_bound_textures[slot] = 0;
    if (!m_encoder)
        return;
    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentTexture:nil atIndex:slot];
    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentSamplerState:nil atIndex:slot];
}

void MetalRenderDevice::generateMipmap(const GpuTexture*) { /* TODO: encode a blit mip generation pass. */ }

auto MetalRenderDevice::createStructuredBuffer(const GpuBufferDesc&) -> Result<Ref<GpuStructuredBuffer>> { return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal structured buffers are not implemented yet"); }

void MetalRenderDevice::updateBufferData(const GpuBufferDesc&, const GpuStructuredBuffer*) {}

void MetalRenderDevice::bindStructuredBuffer(int, const GpuStructuredBuffer*) {}

void MetalRenderDevice::unbindStructuredBuffer(int) {}

void MetalRenderDevice::bindStructuredBufferSRV(int s, const GpuStructuredBuffer* b) { bindStructuredBuffer(s, b); }

void MetalRenderDevice::unbindStructuredBufferSRV(int s) { unbindStructuredBuffer(s); }

void MetalRenderDevice::bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) {
    if (slot >= m_bound_uavs.size())
        return;
    m_bound_uavs[slot] = texture ? texture->GetHandle() : 0;
}

void MetalRenderDevice::unbindUnorderedAccessView(uint32_t slot) {
    if (slot < m_bound_uavs.size())
        m_bound_uavs[slot] = 0;
}

void MetalRenderDevice::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    auto* pso = static_cast<MetalPipelineState*>(m_current_pso);
    if (!pso || !pso->state || !m_command_buffer)
        return;
    id<MTLCommandBuffer> cb = (__bridge id<MTLCommandBuffer>)m_command_buffer;
    id<MTLComputeCommandEncoder> encoder = [cb computeCommandEncoder];
    if (!encoder)
        return;
    id<MTLComputePipelineState> state = (__bridge id<MTLComputePipelineState>)pso->state;
    [encoder setComputePipelineState:state];
    for (NSUInteger i = 0; i < m_bound_textures.size(); ++i) {
        if (m_bound_textures[i])
            [encoder setTexture:(__bridge id<MTLTexture>)reinterpret_cast<void*>(m_bound_textures[i]) atIndex:i];
    }
    for (NSUInteger i = 0; i < m_bound_uavs.size(); ++i) {
        if (m_bound_uavs[i])
            [encoder setTexture:(__bridge id<MTLTexture>)reinterpret_cast<void*>(m_bound_uavs[i]) atIndex:i];
    }
    const MTLSize groups = MTLSizeMake(x, y, z);
    const NSUInteger width = std::max<NSUInteger>(1, std::min<NSUInteger>(state.threadExecutionWidth, state.maxTotalThreadsPerThreadgroup));
    [encoder dispatchThreadgroups:groups threadsPerThreadgroup:MTLSizeMake(width, 1, 1)];
    [encoder endEncoding];
}

void MetalRenderDevice::render() {
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);
    if (w <= 0 || h <= 0)
        return;
    CAMetalLayer* layer = (__bridge CAMetalLayer*)m_layer;
    layer.drawableSize = CGSizeMake(w, h);
    id<CAMetalDrawable> drawable = [layer nextDrawable];
    if (!drawable)
        return;
    m_drawable = (__bridge_retained void*)drawable;
    MTLRenderPassDescriptor* pd = [MTLRenderPassDescriptor renderPassDescriptor];
    pd.colorAttachments[0].texture = drawable.texture;
    pd.colorAttachments[0].loadAction = MTLLoadActionClear;
    pd.colorAttachments[0].storeAction = MTLStoreActionStore;
    pd.colorAttachments[0].clearColor = MTLClearColorMake(0.08, 0.08, 0.1, 1.0);
    id<MTLCommandBuffer> cb = (__bridge id<MTLCommandBuffer>)m_command_buffer;
    id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:pd];
    if (enc && m_app->specification().enableImgui) {
        ImGui_ImplMetal_NewFrame(pd);
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, enc);

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        [enc endEncoding];
    }
}

void MetalRenderDevice::present() {
    id<MTLCommandBuffer> cb = (__bridge id<MTLCommandBuffer>)m_command_buffer;
    if (m_drawable)
        [cb presentDrawable:(__bridge id<CAMetalDrawable>)m_drawable];
    [cb commit];
    if (m_drawable) {
        CFRelease(m_drawable);
        m_drawable = nullptr;
    }
    if (m_command_buffer) {
        CFRelease(m_command_buffer);
        m_command_buffer = nullptr;
    }
}

void MetalRenderDevice::onWindowResize(int, int) {}

} // namespace cave::render
