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

    // @HACK: overlay requires a scene
    // this should be changed at some point
    // create a dummy scene to trigger render graph

    auto scene = MakeOwner<Scene>();
    scene->update(0.0f);

    m_preview_scene = m_scene_reg.registerScene(
        {
            .source = SceneSource::Editor,
            .debug_name = m_handle.meta()->name,
        },
        std::move(scene));
}

}  // namespace cave
