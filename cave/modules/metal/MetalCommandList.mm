#include "MetalCommandList.h"

#include "MetalHelpers.h"
#include "MetalPipelineStateManager.h"
#include "MetalResources.h"

#import <Metal/Metal.h>

#include <algorithm>

namespace cave::render {

void MetalCommandList::setCommandBuffer(void* command_buffer) {
    DEV_ASSERT(!m_encoder);

    m_command_buffer = command_buffer;
}

void MetalCommandList::resetCommandBuffer() {
    endRendering();

    m_command_buffer = nullptr;

    // These are command-list state, not frame resource ownership.
    m_current_mesh = nullptr;
    m_current_pso = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
// Rendering
//---------------------------------------------------------------------------------------------------------------------

static inline MTLLoadAction ConvertLoadOp(LoadOp op) {
    switch (op) {
    case LoadOp::Clear:
        return MTLLoadActionClear;
    case LoadOp::Load:
        return MTLLoadActionLoad;
    default:
        return MTLLoadActionDontCare;
    }
}

void MetalCommandList::beginRendering(const RenderingInfo& info) {
    DEV_ASSERT(!m_encoder);
    DEV_ASSERT(m_command_buffer);

    MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];

    // Color
    for (NSUInteger i = 0; i < info.colors.size() && i < 8; ++i) {
        const auto& color = info.colors[i];
        if (!color.tex)
            continue;

        const auto* texture = static_cast<const MetalTexture*>(color.tex.get());
        DEV_ASSERT(texture && texture->object);

        MTLRenderPassColorAttachmentDescriptor* attachment = pass.colorAttachments[i];

        attachment.texture = (__bridge id<MTLTexture>)texture->object;
        attachment.level = color.view.mip_slice;
        attachment.slice = color.view.first_array_slice;
        attachment.loadAction = ConvertLoadOp(color.load);
        attachment.storeAction = MTLStoreActionStore;
        attachment.clearColor = MTLClearColorMake(color.clear_color[0], color.clear_color[1], color.clear_color[2], color.clear_color[3]);
    }

    // Depth / stencil
    if (info.depth && info.depth->tex) {
        const auto* texture = static_cast<const MetalTexture*>(info.depth->tex.get());
        DEV_ASSERT(texture && texture->object);

        id<MTLTexture> metal_texture = (__bridge id<MTLTexture>)texture->object;
        pass.depthAttachment.texture = metal_texture;
        pass.depthAttachment.level = info.depth->view.mip_slice;
        pass.depthAttachment.slice = info.depth->view.first_array_slice;
        pass.depthAttachment.loadAction = ConvertLoadOp(info.depth->depth_load);
        pass.depthAttachment.storeAction = MTLStoreActionStore;
        pass.depthAttachment.clearDepth = info.depth->clear_depth;

        const PixelFormat format = info.depth->tex->desc.format;

        if (HasStencil(format) || format == PixelFormat::R32G8X24_TYPELESS) {
            pass.stencilAttachment.texture = metal_texture;
            pass.stencilAttachment.level = info.depth->view.mip_slice;
            pass.stencilAttachment.slice = info.depth->view.first_array_slice;
            pass.stencilAttachment.loadAction = ConvertLoadOp(info.depth->stencil_load);
            pass.stencilAttachment.storeAction = MTLStoreActionStore;
            pass.stencilAttachment.clearStencil = info.depth->clear_stencil;
        }
    }

    id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)m_command_buffer;
    id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:pass];
    DEV_ASSERT(encoder);
    m_encoder = encoder;

    //-----------------------------------------------------------------------------------------------------------------
    // Global samplers
    //-----------------------------------------------------------------------------------------------------------------

#define SAMPLER_STATE(REG, NAME, DESC)                                                               \
    if (m_samplers[REG]) {                                                                           \
        [encoder setFragmentSamplerState:(__bridge id<MTLSamplerState>)m_samplers[REG] atIndex:REG]; \
    }

#include "sampler.slang.h"

#undef SAMPLER_STATE

    //-----------------------------------------------------------------------------------------------------------------
    // Constant buffers
    //
    // Transitional behavior matching the old MetalRenderDevice:
    // every encoder receives all global CB bindings.
    //-----------------------------------------------------------------------------------------------------------------

    for (uint32_t slot = 0; slot < m_constant_buffers.size(); ++slot) {
        void* buffer = m_constant_buffers[slot];

        if (!buffer)
            continue;

        id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer;
        [encoder setVertexBuffer:metal_buffer offset:0 atIndex:slot];
        [encoder setFragmentBuffer:metal_buffer offset:0 atIndex:slot];
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Restore texture bindings because Metal encoder state does not survive encoder switches.
    //-----------------------------------------------------------------------------------------------------------------

    for (uint32_t slot = 0; slot < m_bound_textures.size(); ++slot) {
        if (!m_bound_textures[slot])
            continue;

        id<MTLTexture> texture = (__bridge id<MTLTexture>)reinterpret_cast<void*>(m_bound_textures[slot]);
        [encoder setFragmentTexture:texture atIndex:slot];
    }
}

void MetalCommandList::endRendering() {
    if (m_encoder) {
        id<MTLRenderCommandEncoder> encoder = (id<MTLRenderCommandEncoder>)m_encoder;

        [encoder endEncoding];
        m_encoder = nullptr;
    }
}

void MetalCommandList::setViewport(const Viewport& viewport) {
    DEV_ASSERT(m_encoder);

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_encoder;

    MTLViewport vp{};
    vp.originX = static_cast<double>(viewport.top_left_x);
    vp.originY = static_cast<double>(viewport.top_left_y);
    vp.width = static_cast<double>(viewport.width);
    vp.height = static_cast<double>(viewport.height);
    vp.znear = 0.0;
    vp.zfar = 1.0;

    [encoder setViewport:vp];
}

//---------------------------------------------------------------------------------------------------------------------
// Pipeline
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::setPipelineState(PipelineStateName name) {
    m_current_pso = m_pipeline_state_manager.findPSO(name);
    DEV_ASSERT(m_current_pso);
    applyGraphicsPipeline();
}

void MetalCommandList::applyGraphicsPipeline() {
    if (!m_encoder || !m_current_pso) {
        return;
    }

    auto* pipeline = static_cast<MetalPipelineState*>(m_current_pso);

    if (!pipeline->state || pipeline->desc.type != PipelineStateType::GRAPHICS) {
        return;
    }

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_encoder;

    [encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)pipeline->state];

    if (pipeline->depth_state) {
        [encoder setDepthStencilState:(__bridge id<MTLDepthStencilState>)pipeline->depth_state];
    }

    [encoder setStencilReferenceValue:m_stencil_ref];

    if (pipeline->desc.rasterizer_desc) {
        MTLCullMode cull_mode = MTLCullModeNone;

        switch (pipeline->desc.rasterizer_desc->cullMode) {
        case CullMode::FRONT:
            cull_mode = MTLCullModeFront;
            break;
        case CullMode::BACK:
            cull_mode = MTLCullModeBack;
            break;
        default:
            break;
        }

        [encoder setCullMode:cull_mode];

        [encoder setFrontFacingWinding:pipeline->desc.rasterizer_desc->frontCounterClockwise ? MTLWindingCounterClockwise : MTLWindingClockwise];
    }
}

void MetalCommandList::setStencilRef(uint32_t ref) {
    m_stencil_ref = ref;

    if (!m_encoder)
        return;

    [(id<MTLRenderCommandEncoder>)m_encoder setStencilReferenceValue:ref];
}

//---------------------------------------------------------------------------------------------------------------------
// Mesh / IA
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::setMesh(const GpuMesh* mesh) {
    m_current_mesh = mesh;

    applyMesh();
}

void MetalCommandList::applyMesh() {
    if (!m_encoder)
        return;

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_encoder;

    if (!m_current_mesh) {
        for (NSUInteger slot = METAL_VERTEX_BUFFER_BASE; slot < 31; ++slot) {

            [encoder setVertexBuffer:nil offset:0 atIndex:slot];
        }

        return;
    }

    const auto* mesh = static_cast<const MetalGpuMesh*>(m_current_mesh);

    //-----------------------------------------------------------------------------------------------------------------
    // Your current renderer expects all vertex streams to exist in Metal.
    // Missing streams use the first valid VB as a dummy buffer.
    //-----------------------------------------------------------------------------------------------------------------

    const MetalBuffer* dummy_buffer = nullptr;

    for (const auto& vertex_buffer : mesh->vertexBuffers) {
        if (!vertex_buffer)
            continue;

        dummy_buffer = static_cast<const MetalBuffer*>(vertex_buffer.get());
        break;
    }

    if (!dummy_buffer)
        return;

    id<MTLBuffer> dummy = (__bridge id<MTLBuffer>)dummy_buffer->object;

    for (uint32_t slot = 0; slot < mesh->vertexBuffers.size(); ++slot) {

        const auto& vertex_buffer = mesh->vertexBuffers[slot];

        id<MTLBuffer> buffer = dummy;

        if (vertex_buffer) {
            const auto* metal_buffer = static_cast<const MetalBuffer*>(vertex_buffer.get());

            buffer = (__bridge id<MTLBuffer>)metal_buffer->object;
        }

        [encoder setVertexBuffer:buffer offset:0 atIndex:METAL_VERTEX_BUFFER_BASE + slot];
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Draw
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::draw(uint32_t count, uint32_t offset) {
    if (!m_encoder)
        return;

    [(id<MTLRenderCommandEncoder>)m_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:offset vertexCount:count];
}

void MetalCommandList::drawInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) {
    if (!m_encoder)
        return;

    [(id<MTLRenderCommandEncoder>)m_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:offset vertexCount:count instanceCount:instance_count];
}

void MetalCommandList::drawIndexed(uint32_t count, uint32_t offset) { drawIndexedInstanced(1, count, offset); }

void MetalCommandList::drawIndexedInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) {
    if (!m_encoder || !m_current_mesh) {
        return;
    }

    const auto* mesh = static_cast<const MetalGpuMesh*>(m_current_mesh);

    if (!mesh->indexBuffer)
        return;

    const auto* index_buffer = static_cast<const MetalBuffer*>(mesh->indexBuffer.get());

    id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)index_buffer->object;

    [(id<MTLRenderCommandEncoder>)m_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:buffer indexBufferOffset:offset * sizeof(uint32_t) instanceCount:instance_count];
}

//---------------------------------------------------------------------------------------------------------------------
// Constant buffers
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::bindConstantBufferRange(const GpuConstantBuffer* base, uint32_t size, uint32_t offset) {
    DEV_ASSERT(base);

    const auto* buffer = static_cast<const MetalConstantBuffer*>(base);

    DEV_ASSERT(buffer->object);

    id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->object;

    DEV_ASSERT(static_cast<uint64_t>(size) + static_cast<uint64_t>(offset) <= metal_buffer.length);

    if (!m_encoder)
        return;

    const uint32_t slot = base->GetSlot();

    [(id<MTLRenderCommandEncoder>)m_encoder setVertexBuffer:metal_buffer offset:offset atIndex:slot];

    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentBuffer:metal_buffer offset:offset atIndex:slot];
}

//---------------------------------------------------------------------------------------------------------------------
// Textures
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::bindTexture(Dimension, uint64_t handle, int slot) {
    if (slot < 0 || slot >= static_cast<int>(m_bound_textures.size())) {
        return;
    }

    // Keep logical state even when no render encoder currently exists.
    //
    // This is important for compute and for Metal encoder recreation.
    m_bound_textures[slot] = handle;

    if (!m_encoder)
        return;

    id<MTLTexture> texture = nil;

    if (handle) {
        texture = (__bridge id<MTLTexture>)reinterpret_cast<void*>(handle);
    }

    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentTexture:texture atIndex:slot];
}

void MetalCommandList::unbindTexture(Dimension, int slot) {

    if (slot < 0 || slot >= static_cast<int>(m_bound_textures.size())) {
        return;
    }

    m_bound_textures[slot] = 0;

    if (!m_encoder)
        return;

    [(id<MTLRenderCommandEncoder>)m_encoder setFragmentTexture:nil atIndex:slot];
}

//---------------------------------------------------------------------------------------------------------------------
// UAV
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) {

    if (slot >= m_bound_uavs.size())
        return;

    m_bound_uavs[slot] = texture ? texture->GetHandle() : 0;
}

void MetalCommandList::unbindUnorderedAccessView(uint32_t slot) {

    if (slot >= m_bound_uavs.size())
        return;

    m_bound_uavs[slot] = 0;
}

//---------------------------------------------------------------------------------------------------------------------
// Structured buffers
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::bindStructuredBuffer(int, const GpuStructuredBuffer*) {
    // TODO:
    // Implement when Metal structured buffer
    // resources are implemented.
}

void MetalCommandList::unbindStructuredBuffer(int) {}

void MetalCommandList::bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) { bindStructuredBuffer(slot, buffer); }

void MetalCommandList::unbindStructuredBufferSRV(int slot) { unbindStructuredBuffer(slot); }

//---------------------------------------------------------------------------------------------------------------------
// Compute
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::dispatch(uint32_t x, uint32_t y, uint32_t z) {
    if (!m_current_pso || !m_command_buffer) {
        return;
    }

    auto* pipeline = static_cast<MetalPipelineState*>(m_current_pso);

    if (!pipeline->state || pipeline->desc.type != PipelineStateType::COMPUTE) {
        return;
    }

    // Metal cannot have a render encoder and compute encoder active simultaneously.
    endRendering();

    id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)m_command_buffer;

    id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

    if (!encoder)
        return;

    id<MTLComputePipelineState> state = (__bridge id<MTLComputePipelineState>)pipeline->state;

    [encoder setComputePipelineState:state];

    //-----------------------------------------------------------------------------------------------------------------
    // SRVs
    //-----------------------------------------------------------------------------------------------------------------

    for (NSUInteger slot = 0; slot < m_bound_textures.size(); ++slot) {

        if (!m_bound_textures[slot])
            continue;

        id<MTLTexture> texture = (__bridge id<MTLTexture>)reinterpret_cast<void*>(m_bound_textures[slot]);

        [encoder setTexture:texture atIndex:slot];
    }

    //-----------------------------------------------------------------------------------------------------------------
    // UAVs
    //-----------------------------------------------------------------------------------------------------------------

    for (NSUInteger slot = 0; slot < m_bound_uavs.size(); ++slot) {

        if (!m_bound_uavs[slot])
            continue;

        id<MTLTexture> texture = (__bridge id<MTLTexture>)reinterpret_cast<void*>(m_bound_uavs[slot]);

        [encoder setTexture:texture atIndex:slot];
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Dispatch
    //-----------------------------------------------------------------------------------------------------------------

    const MTLSize group_count = MTLSizeMake(x, y, z);

    const NSUInteger thread_width = std::max<NSUInteger>(1, std::min<NSUInteger>(state.threadExecutionWidth, state.maxTotalThreadsPerThreadgroup));

    const MTLSize threads_per_group = MTLSizeMake(thread_width, 1, 1);

    [encoder dispatchThreadgroups:group_count threadsPerThreadgroup:threads_per_group];

    [encoder endEncoding];
}

//---------------------------------------------------------------------------------------------------------------------
// Mips
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::generateMipmap(const GpuTexture* base) {
    if (!base || !m_command_buffer) {
        return;
    }

    const auto* texture = static_cast<const MetalTexture*>(base);

    DEV_ASSERT(texture->object);

    id<MTLTexture> metal_texture = (__bridge id<MTLTexture>)texture->object;

    if (metal_texture.mipmapLevelCount <= 1)
        return;

    // Metal cannot have a render encoder and blit encoder active simultaneously.
    endRendering();

    id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)m_command_buffer;

    id<MTLBlitCommandEncoder> blit = [command_buffer blitCommandEncoder];

    if (!blit)
        return;

    [blit generateMipmapsForTexture:metal_texture];

    [blit endEncoding];
}

//---------------------------------------------------------------------------------------------------------------------
// Debug markers
//---------------------------------------------------------------------------------------------------------------------

void MetalCommandList::pushDebugGroup(std::string_view event) {
    if (m_command_buffer) {
        NSString* name = [[NSString alloc] initWithBytes:event.data() length:event.size() encoding:NSUTF8StringEncoding];
        [(__bridge id<MTLCommandBuffer>)m_command_buffer pushDebugGroup:name];
    }
}

void MetalCommandList::popDebugGroup() {
    if (m_command_buffer) {
        [(__bridge id<MTLCommandBuffer>)m_command_buffer popDebugGroup];
    }
}

} // namespace cave::render
