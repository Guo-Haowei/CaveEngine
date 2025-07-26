#include "default_textures.h"
#include "primitive_meshes.h"

#include "engine/assets/blob_asset.h"
#include "engine/assets/material_asset.h"
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

static void RegisterPersistentMaterials() {
    auto material = std::make_shared<MaterialAsset>();
    material->base_color = Vector4f::One;
    material->metallic = 0.3f;
    material->roughness = 0.7f;
    material->textures[TEXTURE_BASE] = TO_GUID(GUID3);
}

static void RegisterPersistentMeshes() {
    auto& asset_registry = AssetRegistry::GetSingleton();
    auto& graphics_manager = IGraphicsManager::GetSingleton();
    {
        auto mesh = CreatePlaneMesh(Vector3f(0.5f));
        asset_registry.RegisterPersistentAsset("meshes/plane",
                                               TO_GUID(GUID4),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateCubeMesh(Vector3f(0.5f));
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

void RegisterAllPersistentAssets() {
    RegisterPersistentFonts();
    RegisterPersistentImages();
    RegisterPersistentMaterials();
    RegisterPersistentMeshes();
}

}  // namespace cave