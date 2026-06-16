#include "default_textures.h"
#include "primitive_meshes.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave {

extern unsigned char DroidSans_ttf[];
extern int DroidSans_ttf_len;
extern unsigned char fa_solid_900_ttf[];
extern int fa_solid_900_ttf_len;

static constexpr const char GUID1[] = "00000000-0000-0000-0000000000000001";
static constexpr const char GUID2[] = "00000000-0000-0000-0000000000000002";
static constexpr const char GUID3[] = "00000000-0000-0000-0000000000000003";
static constexpr const char GUID4[] = "00000000-0000-0000-0000000000000004";
static constexpr const char GUID5[] = "00000000-0000-0000-0000000000000005";
static constexpr const char GUID6[] = "00000000-0000-0000-0000000000000006";
static constexpr const char GUID7[] = "00000000-0000-0000-0000000000000007";
static constexpr const char GUID8[] = "00000000-0000-0000-0000000000000008";
static constexpr const char GUID9[] = "00000000-0000-0000-0000000000000009";
static constexpr const char GUID10[] = "00000000-0000-0000-0000000000000010";

#define TO_GUID(x) (Guid::Parse(x, sizeof(x) - 1).unwrap())

static AssetRef LoadBlob(const unsigned char* p_data, unsigned int p_length) {
    auto blob = std::make_shared<BlobAsset>();

    std::vector<char> data;
    data.resize(p_length);
    memcpy(data.data(), p_data, p_length);
    blob->SetBlob(std::move(data));
    return blob;
}

static void RegisterPersistentFonts(AppServices& services) {
    auto& asset_reg = services.assetRegistry();

    asset_reg.RegisterPersistentAsset("fonts/DroidSans.ttf",
                                      TO_GUID(GUID1),
                                      LoadBlob(DroidSans_ttf, DroidSans_ttf_len));
    asset_reg.RegisterPersistentAsset("fonts/fa-solid-900.ttf",
                                      TO_GUID(GUID2),
                                      LoadBlob(fa_solid_900_ttf, fa_solid_900_ttf_len));
}

static void RegisterPersistentImages(AppServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto& graphics_manager = services.renderDevice();
    {
        auto texture = CreateCheckerBoardImage();
        asset_registry.RegisterPersistentAsset("textures/checkerboard",
                                               TO_GUID(GUID3),
                                               texture);
        graphics_manager.RequestTexture(texture.get());
    }
}

static void RegisterPersistentMeshes(AppServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto& graphics_manager = services.renderDevice();
    {
        auto mesh = CreatePlaneMesh(Vec3f(0.5f));
        asset_registry.RegisterPersistentAsset("meshes/plane",
                                               TO_GUID(GUID4),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateCubeMesh(Vec3f(0.5f));
        asset_registry.RegisterPersistentAsset("meshes/cube",
                                               TO_GUID(GUID5),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateSphereMesh(0.5f);
        asset_registry.RegisterPersistentAsset("meshes/sphere",
                                               TO_GUID(GUID6),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateCylinderMesh(0.5f, 1.0f);
        asset_registry.RegisterPersistentAsset("meshes/cylinder",
                                               TO_GUID(GUID7),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateConeMesh(0.5f, 1.0f);
        asset_registry.RegisterPersistentAsset("meshes/cone",
                                               TO_GUID(GUID8),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateTorusMesh(0.5f);
        asset_registry.RegisterPersistentAsset("meshes/torus",
                                               TO_GUID(GUID9),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
}

static void RegisterPersistentMaterials(AppServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto material = std::make_shared<MaterialAsset>();
    material->base_color = Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
    asset_registry.RegisterPersistentAsset("materials/default", TO_GUID(GUID10), material);
}

void RegisterAllPersistentAssets(AppServices& services) {
    RegisterPersistentFonts(services);
    RegisterPersistentImages(services);
    RegisterPersistentMaterials(services);
    RegisterPersistentMeshes(services);
}

}  // namespace cave