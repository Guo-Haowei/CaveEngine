#include "MetalRenderDevice.h"

#include "MetalCommandList.h"
#include "MetalHelpers.h"
#include "MetalPipelineStateManager.h"
#include "MetalResources.h"

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

id<MTLSamplerState> createSampler(id<MTLDevice> device, MTLSamplerDescriptor* desc) { return [device newSamplerStateWithDescriptor:desc]; }

} // namespace

//---------------------------------------------------------------------------------------------------------------------
// Resources
//---------------------------------------------------------------------------------------------------------------------

MetalBuffer::~MetalBuffer() {
    if (object) {
        CFRelease(object);
        object = nullptr;
    }
}

MetalTexture::~MetalTexture() {
    if (object) {
        CFRelease(object);
        object = nullptr;
    }
}

MetalConstantBuffer::~MetalConstantBuffer() {
    if (object) {
        CFRelease(object);
        object = nullptr;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Device
//---------------------------------------------------------------------------------------------------------------------

MetalRenderDevice::MetalRenderDevice() : RenderDevice("MetalRenderDevice", Backend::Metal, 1) { m_pipeline_state_manager = MakeOwner<MetalPipelineStateManager>(); }

MetalRenderDevice::~MetalRenderDevice() = default;

auto MetalRenderDevice::InitializeInternal() -> Result<void> {

    @autoreleasepool {
        auto* display = dynamic_cast<GlfwDisplayService*>(&m_app->services().displayService());

        if (!display) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "Metal backend requires GLFW Cocoa display");
        }

        m_window = display->GetGlfwWindow();

        //-----------------------------------------------------------------------------------------------------------------
        // Device
        //-----------------------------------------------------------------------------------------------------------------

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();

        if (!device) {
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "MTLCreateSystemDefaultDevice failed");
        }

        id<MTLCommandQueue> queue = [device newCommandQueue];

        if (!queue) {
            return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Could not create Metal command queue");
        }

        m_device = device;

        m_command_queue = queue;

        //-----------------------------------------------------------------------------------------------------------------
        // CAMetalLayer
        //-----------------------------------------------------------------------------------------------------------------

        NSWindow* window = glfwGetCocoaWindow(m_window);

        NSView* view = window.contentView;

        CAMetalLayer* layer = [CAMetalLayer layer];

        layer.device = device;

        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

        layer.framebufferOnly = YES;

        view.wantsLayer = YES;

        view.layer = layer;

        m_layer = layer;

        //-----------------------------------------------------------------------------------------------------------------
        // ImGui
        //-----------------------------------------------------------------------------------------------------------------

        if (m_app->specification().enableImgui) {
            if (auto* imgui = m_app->services().imgui) {

                auto initialize_cb = [this]() {
                    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

                    ImGui_ImplMetal_Init(device);

                    ImGui_ImplMetal_CreateDeviceObjects(device);
                };

                auto finalize_cb = []() { ImGui_ImplMetal_Shutdown(); };

                imgui->setRenderCallbacks(std::move(initialize_cb), std::move(finalize_cb));
            }
        }

        //-----------------------------------------------------------------------------------------------------------------
        // Static samplers
        //-----------------------------------------------------------------------------------------------------------------

#define SAMPLER_STATE(REG, NAME, DESC)                                                       \
    if (auto sampler = createSampler(device, FillMetalSamplerDesc(DESC)); !sampler) {        \
                                                                                             \
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to create sampler {}", #NAME); \
    } else {                                                                                 \
        m_samplers[REG] = sampler;                                                           \
    }

#include "sampler.slang.h"

#undef SAMPLER_STATE
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Command list
    //-----------------------------------------------------------------------------------------------------------------

    DEV_ASSERT(m_pipeline_state_manager);

    auto& pipeline_manager = dynamic_cast<MetalPipelineStateManager&>(*m_pipeline_state_manager);

    m_command_list = MakeOwner<MetalCommandList>(pipeline_manager, m_samplers, m_constant_buffers);

    // Keep current behavior.
    m_capabilities.supportComputeShaders = false;

    return Result<void>();
}

void MetalRenderDevice::FinalizeImpl() {
    // Command list references sampler / CB arrays,
    // so destroy it first.
    m_command_list.reset();

    if (m_pipeline_state_manager) {
        m_pipeline_state_manager->finalize();
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Samplers
    //-----------------------------------------------------------------------------------------------------------------

    for (void*& sampler : m_samplers) {

        if (!sampler)
            continue;

        CFRelease(sampler);

        sampler = nullptr;
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Native objects
    //-----------------------------------------------------------------------------------------------------------------

    for (void** object : {
             &m_drawable,
             &m_command_buffer,
             &m_layer,
             &m_command_queue,
             &m_device,
         }) {

        if (!*object)
            continue;

        CFRelease(*object);

        *object = nullptr;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Frame
//---------------------------------------------------------------------------------------------------------------------

void MetalRenderDevice::beginFrame() {
    DEV_ASSERT(m_command_queue);
    DEV_ASSERT(m_command_list);

    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_command_queue;

    m_command_buffer = [queue commandBuffer];

    DEV_ASSERT(m_command_buffer);

    m_command_list->setCommandBuffer(m_command_buffer);
}

void MetalRenderDevice::endFrame() {
    if (m_command_list) {
        m_command_list->endRendering();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Transitional command wrappers
//---------------------------------------------------------------------------------------------------------------------
void MetalRenderDevice::generateMipmap(const GpuTexture* texture) {

    DEV_ASSERT(m_command_list);

    m_command_list->generateMipmap(texture);
}

//---------------------------------------------------------------------------------------------------------------------
// Buffers
//---------------------------------------------------------------------------------------------------------------------

auto MetalRenderDevice::createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> {

    const size_t bytes = static_cast<size_t>(desc.element_count) * desc.element_size;

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    id<MTLBuffer> buffer = [device newBufferWithLength:bytes options:MTLResourceStorageModeShared];

    if (!buffer) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal buffer allocation failed ({} bytes)", bytes);
    }

    if (desc.initial_data && bytes) {

        memcpy(buffer.contents, desc.initial_data, bytes);
    }

    auto result = MakeRef<MetalBuffer>(desc);

    result->object = buffer;

    return result;
}

void MetalRenderDevice::updateBuffer(const GpuBufferDesc& desc, GpuBuffer* base) {

    DEV_ASSERT(base);

    auto* buffer = static_cast<MetalBuffer*>(base);

    DEV_ASSERT(buffer->object);

    id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->object;

    const size_t bytes = static_cast<size_t>(desc.element_count) * desc.element_size;

    DEV_ASSERT(bytes <= metal_buffer.length);

    if (desc.initial_data && bytes <= metal_buffer.length) {

        memcpy(metal_buffer.contents, desc.initial_data, bytes);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Mesh
//---------------------------------------------------------------------------------------------------------------------

auto MetalRenderDevice::createMeshImpl(const GpuMeshDesc& desc, std::span<const GpuBufferDesc> vertex_buffers, const GpuBufferDesc* index_buffer) -> Result<Ref<GpuMesh>> {
    auto mesh = MakeRef<MetalGpuMesh>(desc);

    for (uint32_t slot = 0; slot < vertex_buffers.size(); ++slot) {
        const GpuBufferDesc& buffer_desc = vertex_buffers[slot];
        if (!buffer_desc.element_count)
            continue;

        auto result = createBuffer(buffer_desc);
        if (!result) {
            return CAVE_ERROR(result.error());
        }

        mesh->vertexBuffers[slot] = *result;
    }

    if (index_buffer) {
        auto result = createBuffer(*index_buffer);
        if (!result) {
            return CAVE_ERROR(result.error());
        }

        mesh->indexBuffer = *result;
    }

    return mesh;
}

//---------------------------------------------------------------------------------------------------------------------
// Constant buffers
//---------------------------------------------------------------------------------------------------------------------

auto MetalRenderDevice::createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> {
    const size_t bytes = static_cast<size_t>(desc.element_count) * desc.element_size;

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    id<MTLBuffer> buffer = [device newBufferWithLength:bytes options:MTLResourceStorageModeShared];

    if (!buffer) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal constant buffer allocation failed");
    }

    auto result = MakeRef<MetalConstantBuffer>(desc);

    result->object = buffer;

    //-----------------------------------------------------------------------------------------------------------------
    // Transitional fixed-slot model
    //-----------------------------------------------------------------------------------------------------------------

    DEV_ASSERT(desc.slot == m_constant_buffers.size());

    m_constant_buffers.push_back(result->object);

    return result;
}

void MetalRenderDevice::updateConstantBuffer(const GpuConstantBuffer* base, const void* data, size_t size) {
    DEV_ASSERT(base);

    const auto* buffer = static_cast<const MetalConstantBuffer*>(base);

    DEV_ASSERT(buffer->object);

    id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->object;

    DEV_ASSERT(size <= metal_buffer.length);

    if (data) {
        memcpy(metal_buffer.contents, data, size);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Structured buffers
//---------------------------------------------------------------------------------------------------------------------

auto MetalRenderDevice::createStructuredBuffer(const GpuBufferDesc&) -> Result<Ref<GpuStructuredBuffer>> {
    CRASH_NOW();
    return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Metal structured buffers are not implemented yet");
}

void MetalRenderDevice::updateBufferData(const GpuBufferDesc&, const GpuStructuredBuffer*) {}

//---------------------------------------------------------------------------------------------------------------------
// Texture helpers
//---------------------------------------------------------------------------------------------------------------------

static NSUInteger GetBytesPerPixel(MTLPixelFormat format) {

    switch (format) {
    case MTLPixelFormatR8Unorm:
    case MTLPixelFormatR8Uint:
        return 1;

    case MTLPixelFormatRG8Unorm:
    case MTLPixelFormatRG8Uint:
    case MTLPixelFormatR16Float:
        return 2;

    case MTLPixelFormatRGBA8Unorm:
    case MTLPixelFormatRGBA8Unorm_sRGB:
    case MTLPixelFormatBGRA8Unorm:
    case MTLPixelFormatBGRA8Unorm_sRGB:
    case MTLPixelFormatR32Float:
        return 4;

    case MTLPixelFormatRG32Float:
        return 8;

    case MTLPixelFormatRGBA32Float:
        return 16;

    default:
        CRASH_NOW_MSG("Unsupported Metal texture format");

        return 0;
    }
}

Ref<GpuTexture> MetalRenderDevice::createTextureImpl(const GpuTextureDesc& desc, const SamplerDesc&) {
    MTLPixelFormat texture_format = ToMetalTextureFormat(desc.format);

    DEV_ASSERT(texture_format != MTLPixelFormatInvalid);

    if (desc.width == 0 || desc.height == 0) {
        return nullptr;
    }

    bool generate_mips = (desc.bindFlags & BIND_SHADER_RESOURCE) != 0;

    if (desc.dimension == Dimension::TextureCube || desc.dimension == Dimension::TextureCubeArray) {
        generate_mips = false;
    } else if (desc.mipLevels == 1) {
        generate_mips = false;
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Depth formats
    //-----------------------------------------------------------------------------------------------------------------

    switch (desc.format) {
    case PixelFormat::D32_FLOAT:
        texture_format = MTLPixelFormatDepth32Float;
        generate_mips = false;
        break;
    case PixelFormat::D24_UNORM_S8_UINT:
        texture_format = MTLPixelFormatDepth24Unorm_Stencil8;
        generate_mips = false;
        break;
    case PixelFormat::D32_FLOAT_S8X24_UINT:
        texture_format = MTLPixelFormatDepth32Float_Stencil8;
        generate_mips = false;
        break;
    default:
        break;
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Mips
    //-----------------------------------------------------------------------------------------------------------------

    auto get_full_mip_count = [](uint32_t width, uint32_t height, uint32_t depth) {
        uint32_t size = std::max({
            width,
            height,
            depth,
        });

        uint32_t levels = 1;

        while (size > 1) {
            size >>= 1;
            ++levels;
        }

        return levels;
    };

    const uint32_t max_mip_levels = get_full_mip_count(desc.width, desc.height, desc.dimension == Dimension::Texture3D ? desc.depth : 1);

    uint32_t mip_levels = desc.mipLevels;

    if (generate_mips || mip_levels == 0) {

        mip_levels = max_mip_levels;
    }

    if (mip_levels > max_mip_levels) {
        return nullptr;
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Descriptor
    //-----------------------------------------------------------------------------------------------------------------

    MTLTextureDescriptor* texture_desc = [[MTLTextureDescriptor alloc] init];

    texture_desc.pixelFormat = texture_format;
    texture_desc.width = desc.width;
    texture_desc.height = desc.height;
    texture_desc.depth = 1;
    texture_desc.arrayLength = 1;
    texture_desc.mipmapLevelCount = mip_levels;
    texture_desc.sampleCount = 1;

    switch (desc.dimension) {
    case Dimension::Texture2D:
        texture_desc.textureType = MTLTextureType2D;
        texture_desc.arrayLength = 1;
        break;

    case Dimension::Texture2DArray:
        if (desc.arraySize == 0)
            return nullptr;

        texture_desc.textureType = MTLTextureType2DArray;
        texture_desc.arrayLength = desc.arraySize;
        break;

    case Dimension::TextureCube:
        if (desc.width != desc.height) {
            return nullptr;
        }

        texture_desc.textureType = MTLTextureTypeCube;
        texture_desc.arrayLength = 1;
        break;

    case Dimension::TextureCubeArray:
        if (desc.width != desc.height) {
            return nullptr;
        }

        if (desc.arraySize == 0 || (desc.arraySize % 6) != 0) {
            return nullptr;
        }

        texture_desc.textureType = MTLTextureTypeCubeArray;

        texture_desc.arrayLength = desc.arraySize / 6;
        break;

    case Dimension::Texture3D:
        if (desc.depth == 0)
            return nullptr;

        texture_desc.textureType = MTLTextureType3D;
        texture_desc.depth = desc.depth;
        texture_desc.arrayLength = 1;
        break;

    default:
        return nullptr;
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Usage
    //-----------------------------------------------------------------------------------------------------------------

    texture_desc.usage = 0;

    if (desc.bindFlags & BIND_SHADER_RESOURCE) {
        texture_desc.usage |= MTLTextureUsageShaderRead;
    }

    if (desc.bindFlags & BIND_UNORDERED_ACCESS) {
        texture_desc.usage |= MTLTextureUsageShaderWrite;
    }

    if ((desc.bindFlags & BIND_RENDER_TARGET) || (desc.bindFlags & BIND_DEPTH_STENCIL)) {
        texture_desc.usage |= MTLTextureUsageRenderTarget;
    }

    if (texture_desc.usage == 0) {
        texture_desc.usage = MTLTextureUsageShaderRead;
    }

    texture_desc.storageMode = MTLStorageModeShared;

    //-----------------------------------------------------------------------------------------------------------------
    // Create
    //-----------------------------------------------------------------------------------------------------------------

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    id<MTLTexture> texture = [device newTextureWithDescriptor:texture_desc];

    if (!texture)
        return nullptr;

    //-----------------------------------------------------------------------------------------------------------------
    // Initial data
    //-----------------------------------------------------------------------------------------------------------------

    if (desc.initialData) {
        const NSUInteger bytes_per_pixel = GetBytesPerPixel(texture_format);

        if (bytes_per_pixel == 0)
            return nullptr;

        const NSUInteger row_pitch = static_cast<NSUInteger>(desc.width) * bytes_per_pixel;

        if (desc.dimension == Dimension::Texture3D) {
            const NSUInteger image_pitch = row_pitch * static_cast<NSUInteger>(desc.height);
            [texture replaceRegion:MTLRegionMake3D(0, 0, 0, desc.width, desc.height, desc.depth) mipmapLevel:0 slice:0 withBytes:desc.initialData bytesPerRow:row_pitch bytesPerImage:image_pitch];
        } else {
            [texture replaceRegion:MTLRegionMake2D(0, 0, desc.width, desc.height) mipmapLevel:0 slice:0 withBytes:desc.initialData bytesPerRow:row_pitch bytesPerImage:0];
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Initial mip generation
    //
    // Leave this synchronous creation path alone for now.
    // Runtime mip generation goes through MetalCommandList.
    //-----------------------------------------------------------------------------------------------------------------

    if ((desc.miscFlags & RESOURCE_MISC_GENERATE_MIPS) && mip_levels > 1 && (desc.bindFlags & BIND_SHADER_RESOURCE)) {
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_command_queue;
        id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
        id<MTLBlitCommandEncoder> blit = [command_buffer blitCommandEncoder];

        [blit generateMipmapsForTexture:texture];
        [blit endEncoding];

        [command_buffer commit];
        // Fine for resource initialization for now.
        [command_buffer waitUntilCompleted];
    }

    auto result = MakeRef<MetalTexture>(desc);
    result->object = texture;

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
// Swapchain / ImGui
//---------------------------------------------------------------------------------------------------------------------

void MetalRenderDevice::render() {
    // RenderGraph render encoder must be closed before
    // creating the backbuffer/ImGui encoder.
    if (m_command_list) {
        m_command_list->endRendering();
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    if (width <= 0 || height <= 0) {
        return;
    }

    CAMetalLayer* layer = (__bridge CAMetalLayer*)m_layer;

    layer.drawableSize = CGSizeMake(width, height);

    id<CAMetalDrawable> drawable = [layer nextDrawable];

    if (!drawable)
        return;

    m_drawable = drawable;

    MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];

    pass.colorAttachments[0].texture = drawable.texture;
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(0.08, 0.08, 0.10, 1.0);

    id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)m_command_buffer;

    id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:pass];

    if (!encoder)
        return;

    if (m_app->specification().enableImgui) {
        ImGui_ImplMetal_NewFrame(pass);

        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, encoder);

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Must always close the encoder,
    // including when ImGui is disabled.
    [encoder endEncoding];
}

void MetalRenderDevice::present() {
    if (!m_command_buffer)
        return;

    id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)m_command_buffer;

    if (m_drawable) {
        [command_buffer presentDrawable:(__bridge id<CAMetalDrawable>)m_drawable];
    }

    [command_buffer commit];

    if (m_command_list) {
        m_command_list->resetCommandBuffer();
    }

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

//---------------------------------------------------------------------------------------------------------------------
// Command list
//---------------------------------------------------------------------------------------------------------------------

ICommandList* MetalRenderDevice::getCommandList() const { return m_command_list.get(); }

} // namespace cave::render
