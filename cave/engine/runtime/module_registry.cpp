#include "module_registry.h"

#include "engine/assets/asset_manager.h"
#include "engine/empty/empty_display_manager.h"
#include "engine/empty/empty_graphics_manager.h"
#include "engine/empty/empty_physics_manager.h"
#include "engine/empty/empty_script_manager.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/scripting/lua/lua_script_manager.h"

#if USING(PLATFORM_WINDOWS)
#include "modules/d3d11/d3d11_graphics_manager.h"
#include "modules/d3d12/d3d12_graphics_manager.h"
#include "modules/opengl4/opengl4_graphics_manager.h"
#include "modules/vk/vulkan_graphics_manager.h"
#elif USING(PLATFORM_APPLE)
#include "engine/drivers/metal/metal_graphics_manager.h"
#elif USING(PLATFORM_WASM)
#include "modules/opengles3/opengles3_graphics_manager.h"
#endif

namespace cave {

template<class T1, class FALLBACK>
inline T1* CreateModule() {
    if (T1::s_createFunc) {
        return T1::s_createFunc();
    }
    return new FALLBACK;
}

class NullSceneManager : public ISceneManager {
public:
    NullSceneManager()
        : ISceneManager("NullSceneManager") {}

    auto InitializeImpl() -> Result<void> override { return Result<void>(); }
    void FinalizeImpl() override {}

    std::shared_ptr<Scene> GetActiveScene() const override { return nullptr; }

    void Update() override {}

    void BumpRevision() override {}
};

IAssetManager* CreateAssetManager() {
    return CreateModule<IAssetManager, AssetManager>();
}

IDisplayManager* CreateDisplayManager() {
    return CreateModule<IDisplayManager, EmptyDisplayManager>();
}

IPhysicsManager* CreatePhysicsManager() {
    return CreateModule<IPhysicsManager, EmptyPhysicsManager>();
}

ISceneManager* CreateSceneManager() {
    return CreateModule<ISceneManager, NullSceneManager>();
}

IScriptManager* CreateScriptManager() {
    return CreateModule<IScriptManager, NullScriptManager>();
}

// @TODO: move to RHI
static IGraphicsManager* SelectGraphicsManager(const std::string& p_backend) {
    if (p_backend == "d3d11") {
#if USING(PLATFORM_WINDOWS)
        return new D3d11GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == "d3d12") {
#if USING(PLATFORM_WINDOWS)
        return new D3d12GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == "opengl") {
#if USING(PLATFORM_WINDOWS)
        return new OpenGL4GraphicsManager;
#elif USING(PLATFORM_WASM)
        return new OpenGLES3GraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == "vulkan") {
#if USING(PLATFORM_WINDOWS)
        return new VulkanGraphicsManager;
#else
        return nullptr;
#endif
    }

    if (p_backend == "metal") {
        return nullptr;
    }

    return new EmptyGraphicsManager;
}

IGraphicsManager* CreateGraphicsManager() {
    if (IGraphicsManager::s_createFunc) {
        return IGraphicsManager::s_createFunc();
    }

    const std::string& backend = DVAR_GET_STRING(gfx_backend);
    IGraphicsManager* manager = SelectGraphicsManager(backend);

    if (!manager) {
        manager = new EmptyGraphicsManager;

        LOG_ERROR("backend '{}' not supported, fallback to EmptyGraphicsManager", backend);
    }

    return manager;
}

}  // namespace cave
