#include "default_textures.h"

#include "engine/runtime/graphics_manager_interface.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

static constexpr const char GUID1[] = "00000000-0000-0000-0000000000000001";

static void RegisterPersistentImages() {
    auto& asset_registry = AssetRegistry::GetSingleton();
    auto& graphics_manager = IGraphicsManager::GetSingleton();
    {
        const Guid CHECKER_BOARD_GUID = Guid::Parse(GUID1, sizeof(GUID1) - 1).value();

        auto texture = CreateCheckerBoardImage();
        asset_registry.RegisterPersistentAsset("textures/checkerboard",
                                               CHECKER_BOARD_GUID,
                                               texture);
        graphics_manager.RequestTexture(texture.get());
    }
}

void RegisterAllPersistentAssets() {
    RegisterPersistentImages();
}

}  // namespace cave