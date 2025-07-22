#include "default_textures.h"

#include "engine/assets/blob_asset.h"
#include "engine/runtime/graphics_manager_interface.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

extern unsigned char DroidSans_ttf[];
extern int DroidSans_ttf_len;
extern unsigned char fa_solid_900_ttf[];
extern int fa_solid_900_ttf_len;

static constexpr const char GUID1[] = "00000000-0000-0000-0000000000000001";
static constexpr const char GUID2[] = "00000000-0000-0000-0000000000000002";
static constexpr const char GUID3[] = "00000000-0000-0000-0000000000000003";

#define TO_GUID(x) (Guid::Parse(x, sizeof(x) - 1).value())

static AssetRef LoadBlob(const unsigned char* p_data, unsigned int p_length) {
    auto blob = std::make_shared<BlobAsset>();

    std::vector<char> data;
    data.resize(p_length);
    memcpy(data.data(), p_data, p_length);
    blob->SetBlob(std::move(data));
    return blob;
}

static void RegisterPersistentFonts() {
    auto& asset_registry = AssetRegistry::GetSingleton();

    {
        asset_registry.RegisterPersistentAsset("fonts/DroidSans.ttf",
                                               TO_GUID(GUID1),
                                               LoadBlob(DroidSans_ttf, DroidSans_ttf_len));
    }
    {
        asset_registry.RegisterPersistentAsset("fonts/fa-solid-900.ttf",
                                               TO_GUID(GUID2),
                                               LoadBlob(fa_solid_900_ttf, fa_solid_900_ttf_len));
    }
}

static void RegisterPersistentImages() {
    auto& asset_registry = AssetRegistry::GetSingleton();
    auto& graphics_manager = IGraphicsManager::GetSingleton();
    {
        auto texture = CreateCheckerBoardImage();
        asset_registry.RegisterPersistentAsset("textures/checkerboard",
                                               TO_GUID(GUID3),
                                               texture);
        graphics_manager.RequestTexture(texture.get());
    }
}

void RegisterAllPersistentAssets() {
    RegisterPersistentFonts();
    RegisterPersistentImages();
}

}  // namespace cave