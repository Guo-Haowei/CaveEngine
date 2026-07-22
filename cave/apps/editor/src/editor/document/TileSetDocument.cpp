#include "TileSetDocument.h"

#include "cave/render/components/SpriteRendererComponent.h"
#include "cave/runtime/assets/Builtin.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

// @TODO: no private include
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

using namespace ::cave::math;

TileSetDocument::TileSetDocument(EngineServices& services, const Guid& guid)
    : DocumentBase(services, guid) {

    const TileSetAsset* tile_set = nullptr;
    if (auto tile_set_handle = m_asset_reg.findByGuid<TileSetAsset>(guid)) {
        tile_set = tile_set_handle.unwrap_unchecked().get();
    }

    DEV_ASSERT(tile_set);

    auto scene = MakeOwner<Scene>();
    auto ent = scene->createEntity();
    scene->create<NameComponent>(ent).setName("Checkerboard");
    scene->create(HierarchyComponent_Id, ent);

    auto& transform = scene->create<TransformComponent>(ent);
    auto& sprite = scene->create<SpriteRendererComponent>(ent);

    const Vec2f size{ tile_set->width(), tile_set->height() };
    transform.setScale(Vec3f(size / TileSetAsset::kDefaultCellSizePx, 1.0f));
    transform.setTranslation(Vec3f(0.5f * size / TileSetAsset::kDefaultCellSizePx, 0.0f));

    sprite.setRect(Box2(Vec2f::Zero, size * (1.0f / CheckerboardInfo::kCellSizePx)));
    sprite.setZIndex(-5);
    sprite.setImageGuid(Guid::parse(std::string_view(builtin::GUID3)).unwrap());

    scene->hierarchy().rebuild(*scene);
    scene->update(0.0f);

    m_preview_scene = m_scene_reg.registerScene(
        {
            .source = SceneSource::Editor,
            .debug_name = m_handle.meta()->name,
        },
        std::move(scene));
}

}  // namespace cave
