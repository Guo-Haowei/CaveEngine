#include "SceneImporter.h"

#include "cave/core/string/StringUtils.h"
#include "cave/runtime/ecs/components/MiscComponents.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/MaterialAsset.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/assets/SceneAsset.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

namespace fs = std::filesystem;
using namespace cave::render;

SceneImporter::SceneImporter(const fs::path& source_path,
                             const fs::path& dest_dir)
    : AssetImporter(source_path, dest_dir) {

    m_file_name = StringUtils::removeExtension(m_source_path.filename().string());

    m_base_path = fs::path(m_source_path).remove_filename().string();

    m_dest_dir = m_dest_dir / m_file_name;
}

Result<void> SceneImporter::PrepareImport() {
    try {
        if (!fs::exists(m_dest_dir)) {
            fs::create_directories(m_dest_dir);
        }
    } catch (const fs::filesystem_error& e) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "{}", e.what());
    }

    return Result<void>();
}

std::string SceneImporter::nameGenerator(std::string_view name, uint32_t& counter) {
    ++counter;
    return std::format("{}_{}", name, counter);
}

Result<Guid> SceneImporter::RegisterImage(const std::filesystem::path& sys_path, bool srgb) {
    fs::path name = sys_path.filename();

    fs::path image_path = m_dest_dir / name;

    std::string virtual_path = IAssetManager::singleton().resolvePath(image_path);
    if (auto res = AssetRegistry::singleton().findByPath<ImageAsset>(virtual_path)) {
        return res.unwrap_unchecked().guid();
    }

    // copy image to dest
    std::error_code err;
    const bool ok = fs::copy_file(sys_path, image_path, std::filesystem::copy_options::overwrite_existing, err);
    if (!ok) {
        return CAVE_ERROR(ErrorCode::FAILURE, "Failed to copy file from {} to {}", sys_path.string(), image_path.string());
    }

    auto _meta = AssetMetaData::createMeta(virtual_path);
    if (_meta.is_none()) {
        CRASH_NOW();
    }

    // save meta
    AssetMetaData meta = std::move(_meta.unwrap_unchecked());

    meta.import_settings["color_space"] = srgb ? "srgb" : "linear";
    meta.import_settings["sampler"] = "linear";

    if (auto res = meta.saveToDisk(nullptr); !res) {
        return CAVE_ERROR(res.error());
    }

    Guid guid = meta.guid;

    AssetRegistry::singleton().registerAsset(std::move(meta), nullptr);

    IAssetManager::singleton().loadAssetSync(guid);

    return guid;
}

Result<Guid> SceneImporter::RegisterMaterial(std::string&& name,
                                             Ref<MaterialAsset>&& material) {

    fs::path sys_path = m_dest_dir / std::format("{}.mat", name);

    Guid guid = Guid::make();
    AssetMetaData meta;
    meta.type = AssetType::Material;
    meta.name = std::move(name);
    meta.guid = guid;
    meta.import_path = IAssetManager::singleton().resolvePath(sys_path);

    if (auto res = material->saveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    AssetRegistry::singleton().registerAsset(std::move(meta), material);

    // @TODO: request textures

    return guid;
}

Result<Guid> SceneImporter::RegisterMesh(std::string&& name,
                                         Ref<MeshAsset>&& mesh) {
    fs::path sys_path = m_dest_dir / std::format("{}.mesh", name);

    Guid guid = Guid::make();
    AssetMetaData meta;
    meta.type = AssetType::Mesh;
    meta.name = std::move(name);
    meta.guid = guid;
    meta.import_path = IAssetManager::singleton().resolvePath(sys_path);

    if (auto res = mesh->saveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    AssetRegistry::singleton().registerAsset(std::move(meta), mesh);

    // @TODO: move it to somewhere else, if it's headless, no need to create gpu data
    RenderDevice::singleton().RequestMesh(mesh.get());

    return Result<Guid>(guid);
}

Result<void> SceneImporter::RegisterScene(ecs::Entity root) {
    Scene& scene = m_scene_asset->sceneMut();

    scene.component<NameComponent>(root)->setName(m_file_name);
    scene.rebuildHierarchy();

    fs::path sys_path = m_dest_dir / std::format("{}.scene", m_file_name);

    AssetMetaData meta;
    meta.type = AssetType::Scene;
    meta.name = m_file_name;
    meta.guid = Guid::make();
    meta.import_path = IAssetManager::singleton().resolvePath(sys_path);
    if (auto res = m_scene_asset->saveToDisk(meta); !res) {
        return CAVE_ERROR(res.error());
    }

    // @TODO: refactor this part
    AssetRegistry::singleton().registerAsset(std::move(meta), m_scene_asset);
    return Result<void>();
}

}  // namespace cave
