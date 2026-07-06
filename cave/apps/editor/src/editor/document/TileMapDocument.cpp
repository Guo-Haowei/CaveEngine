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
    m_preview_scene = m_scene_reg.registerScene(std::move(scene));
}

std::unique_ptr<Scene> TileMapDocument::createPreviewScene() const {
    SceneCommandWriter cb(m_asset_reg);
    Entity root = cb.rootObject();

    Entity ent = cb.tileMapObject("tilemap");
    cb.attachChild(ent, root);
    cb.setProperty(ent, TileMapInstanceComponent_Id, "tile_map_id"_sid, guid());

    auto scene = std::make_unique<Scene>(std::format("preview-tile-map-{}", guid().toString()));

    SceneCommandExecutor executor(*scene);
    EntityMap map(cb.allocationCount());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    scene->setRoot(map.Resolve(root));
#pragma warning(push)
#pragma warning(disable : 4996)
    scene->update(0.0f);
#pragma warning(pop)

    return scene;
}

}  // namespace cave
