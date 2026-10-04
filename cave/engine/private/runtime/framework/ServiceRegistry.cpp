#include "ServiceRegistry.h"

#include "engine/private/runtime/assets/AssetManager.h"
#include "engine/private/runtime/display/NullDisplayService.h"
#include "engine/private/runtime/null/NullRenderDevice.h"
#include "engine/private/runtime/null/NullPhysicsService.h"
#include "engine/private/renderer/graphics_dvars.h"

#if USING(PLATFORM_WINDOWS)
#include "modules/d3d11/d3d11_graphics_manager.h"
#include "modules/d3d12/d3d12_graphics_manager.h"
#include "modules/opengl4/GL4RenderDevice.h"
// #include "modules/vk/vulkan_graphics_manager.h"
#elif USING(PLATFORM_APPLE)
#include "modules/opengl4/GL4RenderDevice.h"
#include "engine/private/drivers/metal/metal_graphics_manager.h"
#elif USING(PLATFORM_WASM)
#include "modules/opengles3/GLES3RenderDevice.h"
#endif

namespace cave {

using namespace cave::render;

template<class T1, class FALLBACK>
inline T1* CreateModule() {
    if (T1::s_createFunc) {
        return T1::s_createFunc();
    }
    return new FALLBACK;
}

IAssetManager* CreateAssetService() {
    return CreateModule<IAssetManager, AssetManager>();
}

DisplayService* CreateDisplayService() {
    return CreateModule<DisplayService, NullDisplayService>();
}

// @TODO: move to RHI
static IRenderDevice* SelectRenderDevice(rhi::Backend p_backend) {
    using rhi::Backend;

    if (p_backend == Backend::Direct3D11) {
#if USING(PLATFORM_WINDOWS)
        return new D3d11GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == Backend::Direct3D12) {
#if USING(PLATFORM_WINDOWS)
        return new D3d12GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == Backend::OpenGL) {
#if USING(PLATFORM_WASM)
        return new OpenGLES3GraphicsManager;
#else
        return new OpenGL4GraphicsManager;
#endif
    }

    if (p_backend == Backend::Vulkan) {
#if USING(PLATFORM_WINDOWS)
        // return new VulkanGraphicsManager;
        return nullptr;
#else
        return nullptr;
#endif
    }

    if (p_backend == Backend::Metal) {
        return nullptr;
    }

    return new NullRenderDevice;
}

IRenderDevice* CreateRenderDevice(rhi::Backend p_backend) {
    if (IRenderDevice::s_createFunc) {
        return IRenderDevice::s_createFunc();
    }

    IRenderDevice* device = SelectRenderDevice(p_backend);

    if (!device) {
        device = new NullRenderDevice;

        LOG_ERROR("backend '{}' not supported, fallback to NullRenderDevice", (int)p_backend);
    }

    return device;
}

}  // namespace cave
