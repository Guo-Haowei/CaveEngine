#include "ServiceRegistry.h"

#include "engine/private/runtime/assets/AssetManager.h"
#include "engine/private/runtime/display/NullDisplayService.h"
#include "engine/private/runtime/null/NullRenderDevice.h"
#include "engine/private/runtime/null/NullPhysicsService.h"
#include "engine/private/renderer/graphics_dvars.h"

#if USING(PLATFORM_WINDOWS)
#include "modules/d3d11/D3d11RenderDevice.h"
#include "modules/d3d12/d3d12_graphics_manager.h"
#include "modules/opengl4/OpenGl4RenderDevice.h"
#elif USING(PLATFORM_APPLE)
#include "modules/metal/MetalRenderDevice.h"
#include "modules/opengl4/OpenGl4RenderDevice.h"
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
static IRenderDevice* SelectRenderDevice(rhi::Backend backend) {
    using rhi::Backend;

    if (backend == Backend::Direct3D11) {
#if USING(PLATFORM_WINDOWS)
        return new D3d11RenderDevice;
#else
        return nullptr;
#endif
    }

    if (backend == Backend::Direct3D12) {
#if USING(PLATFORM_WINDOWS)
        return new D3d12GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (backend == Backend::OpenGL) {
#if USING(PLATFORM_WASM)
        return new GLES3RenderDevice;
#else
        return new GL4RenderDevice;
#endif
    }

#if USING(PLATFORM_WINDOWS)
    if (backend == Backend::Vulkan) {
        return nullptr;
    }
#endif

#if USING(PLATFORM_APPLE)
    if (backend == Backend::Metal) {
        return new MetalRenderDevice;
    }
#endif

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
