#include "TileMapDocument.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

// @TODO: remove private #include
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneCommandExecutor.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using namespace ::cave::literals;
using ecs::Entity;

TileMapDocument::TileMapDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    auto scene = createPreviewScene();
    preview_scene_ = scene_reg_.registerScene(std::move(scene));
}

std::unique_ptr<Scene> TileMapDocument::createPreviewScene() const {
    SceneCommandWriter cb(asset_reg_);
    Entity root = cb.CreateRootObject();

    Entity ent = cb.CreateTileMapObject("tilemap");
    cb.AttachChild(ent, root);
    cb.SetProperty(ent, TileMapInstanceComponent_Id, "tile_map_id"_sid, guid());

    auto scene = std::make_unique<Scene>(std::format("preview-tile-map-{}", guid().toString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.GetAllocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->setRoot(map.Resolve(root));
    scene->update(0.0f);

    return scene;
}

}  // namespace cave
