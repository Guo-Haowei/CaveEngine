#include "DefaultTextures.h"
#include "PrimitiveMeshes.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave {

using namespace ::cave::builtin;

extern unsigned char DroidSans_ttf[];
extern int DroidSans_ttf_len;
extern unsigned char fa_solid_900_ttf[];
extern int fa_solid_900_ttf_len;

#define TO_GUID(x) (Guid::parse(x, sizeof(x) - 1).unwrap())

static AssetRef LoadBlob(const unsigned char* p_data, unsigned int p_length) {
    auto blob = std::make_shared<BlobAsset>();

    std::vector<char> data;
    data.resize(p_length);
    memcpy(data.data(), p_data, p_length);
    blob->SetBlob(std::move(data));
    return blob;
}

static void RegisterPersistentFonts(EngineServices& services) {
    auto& asset_reg = services.assetRegistry();

    asset_reg.registerPersistentAsset("fonts/DroidSans.ttf",
                                      TO_GUID(GUID1),
                                      LoadBlob(DroidSans_ttf, DroidSans_ttf_len));
    asset_reg.registerPersistentAsset("fonts/fa-solid-900.ttf",
                                      TO_GUID(GUID2),
                                      LoadBlob(fa_solid_900_ttf, fa_solid_900_ttf_len));
}

static void RegisterPersistentImages(EngineServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto& graphics_manager = services.renderDevice();
    {
        auto texture = CreateCheckerBoardImage();
        asset_registry.registerPersistentAsset("textures/checkerboard",
                                               TO_GUID(GUID3),
                                               texture);
        graphics_manager.RequestTexture(texture.get());
    }
    {
        auto texture = CreateWhite1x1Image();
        asset_registry.registerPersistentAsset("textures/white@1x1",
                                               TO_GUID(GUID11),
                                               texture);
        graphics_manager.RequestTexture(texture.get());
    }
}

static void RegisterPersistentMeshes(EngineServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto& graphics_manager = services.renderDevice();
    {
        auto mesh = CreatePlaneMesh(Vec3f(0.5f));
        asset_registry.registerPersistentAsset("meshes/plane",
                                               TO_GUID(GUID4),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateCubeMesh(Vec3f(0.5f));
        asset_registry.registerPersistentAsset("meshes/cube",
                                               TO_GUID(GUID5),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateSphereMesh(0.5f);
        asset_registry.registerPersistentAsset("meshes/sphere",
                                               TO_GUID(GUID6),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateCylinderMesh(0.5f, 1.0f);
        asset_registry.registerPersistentAsset("meshes/cylinder",
                                               TO_GUID(GUID7),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateConeMesh(0.5f, 1.0f);
        asset_registry.registerPersistentAsset("meshes/cone",
                                               TO_GUID(GUID8),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
    {
        auto mesh = CreateTorusMesh(0.5f);
        asset_registry.registerPersistentAsset("meshes/torus",
                                               TO_GUID(GUID9),
                                               mesh);
        graphics_manager.RequestMesh(mesh.get());
    }
}

static void RegisterPersistentMaterials(EngineServices& services) {
    auto& asset_registry = services.assetRegistry();
    auto material = std::make_shared<MaterialAsset>();
    material->base_color = Vec4f(1.0f, 0.0f, 1.0f, 1.0f);
    asset_registry.registerPersistentAsset("materials/default", TO_GUID(GUID10), material);
}

void RegisterAllPersistentAssets(EngineServices& services) {
    RegisterPersistentFonts(services);
    RegisterPersistentImages(services);
    RegisterPersistentMaterials(services);
    RegisterPersistentMeshes(services);
}

}  // namespace cave