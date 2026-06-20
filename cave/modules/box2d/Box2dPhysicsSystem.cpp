#include "Box2dPhysicsSystem.h"

#include <box2d/box2d.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

// @TODO: refactor
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace cave::math;

static_assert(sizeof(b2WorldId) == sizeof(uint32_t));
static_assert(sizeof(b2BodyId) == sizeof(uint64_t));

static b2WorldId GetWorldId(uint32_t p_raw_id) {
    return std::bit_cast<b2WorldId>(p_raw_id);
}

Box2dPhysicsSystem::Box2dPhysicsSystem()
    : debug_id_(MakeDebugId(this)) {
}

void Box2dPhysicsSystem::update(float dt) {
    constexpr int sub_step_count = 4;

    if (world_id_.is_none()) {
        return;
    }

    b2WorldId world_id = GetWorldId(world_id_.unwrap_unchecked());
    Scene& scene = context().scene;

    // 1. set speed
    {
        auto view = scene.view<ColliderComponent, VelocityComponent>();
        for (auto [id, collider, vel] : view) {
            b2BodyId body_id = std::bit_cast<b2BodyId>(collider.user_data_);
            b2Body_SetLinearVelocity(body_id, { vel.linear.x, vel.linear.y });
        }
    }

    // 2. simulate
    b2World_Step(world_id, dt, sub_step_count);

    // 3. sync speed and position
    auto view = scene.view<ColliderComponent, TransformComponent>();
    for (auto [id, collider, transform] : view) {
        b2BodyId body_id = std::bit_cast<b2BodyId>(collider.user_data_);

        b2Vec2 position = b2Body_GetPosition(body_id);
        [[maybe_unused]] b2Rot rotation = b2Body_GetRotation(body_id);

        Vec3f translation = transform.GetTranslation();
        translation.x = position.x;
        translation.y = position.y;
        transform.SetTranslation(translation);
        transform.SetDirty();

        if (VelocityComponent* vel = scene.component<VelocityComponent>(id); vel) {
            b2Vec2 linear = b2Body_GetLinearVelocity(body_id);
            vel->linear.x = linear.x;
            vel->linear.y = linear.y;
        }
    }
}

void Box2dPhysicsSystem::onAttach() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, -20.0f };
    b2WorldId world_id = b2CreateWorld(&worldDef);

    world_id_ = Some(std::bit_cast<uint32_t>(world_id));

    Scene& scene = context().scene;

    for (auto [id, collider, transform] : scene.view<ColliderComponent, TransformComponent>()) {
        Vec4f position = transform.GetWorldMatrix() * Vec4f::UnitW;
        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.position = { position.x, position.y };
        body_def.fixedRotation = true;
#if USING(DEBUG_BUILD)
        const NameComponent* name = scene.component<NameComponent>(id);
        if (name) {
            body_def.name = name->GetName().data();
        }
#endif

        b2ShapeDef shape_def = b2DefaultShapeDef();

        switch (collider.bodyType()) {
            case BodyType::Static: {
                body_def.type = b2_staticBody;
            } break;
            case BodyType::Kinematic: {
                body_def.type = b2_kinematicBody;
            } break;
            case BodyType::Dynamic: {
                body_def.type = b2_dynamicBody;
                // @TODO: editor support
                shape_def.density = 1.0f;
                shape_def.material.friction = 0.0f;
            } break;
        }

        b2BodyId body_id = b2CreateBody(world_id, &body_def);

        const Shape& shape = collider.shape();
        switch (shape.type) {
            case ShapeType::Box: {
                const auto& half = shape.data.half;
                b2Polygon box = b2MakeBox(half.x, half.y);
                b2CreatePolygonShape(body_id, &shape_def, &box);

                collider.user_data_ = std::bit_cast<size_t>(body_id);
            } break;
            default:
                break;
        }
    }

    for (auto [id, tile_map_renderer, transform] : scene.view<TileMapInstanceComponent, TransformComponent>()) {
        const TileMapAsset* tile_map = tile_map_renderer.tileMapHandle().Get();
        if (!tile_map) continue;
        const TileSetAsset* tile_set = tile_map->tileSetHandle().Get();
        if (!tile_set) continue;

        Vec4f position = transform.GetWorldMatrix() * Vec4f::UnitW;

        for (const auto& [key, chunk] : tile_map->tiles().chunks()) {
            const int16_t offset_x = key.x * kTileChunkSize;
            const int16_t offset_y = key.y * kTileChunkSize;
            for (int16_t y = offset_y; y < offset_y + kTileChunkSize; ++y) {
                for (int16_t x = offset_x; x < offset_x + kTileChunkSize; ++x) {
                    const TileId& tile_id = chunk->at(x - offset_x, y - offset_y);
                    auto res = tile_set->getCollider(tile_id);
                    if (res.is_none()) continue;
                    Shape shape = res.unwrap_unchecked();
                    DEV_ASSERT(shape.type == ShapeType::Box);

                    // @TODO: fix this part
                    b2BodyDef body_def = b2DefaultBodyDef();
                    body_def.type = b2_staticBody;
                    body_def.position = {
                        position.x + x - 0.5f,
                        position.y + y + 0.5f,
                    };
                    body_def.fixedRotation = true;

                    b2BodyId body_id = b2CreateBody(world_id, &body_def);
                    const auto& half = shape.data.half;
                    b2Polygon box = b2MakeBox(half.x, half.y);
                    b2ShapeDef shape_def = b2DefaultShapeDef();
                    b2CreatePolygonShape(body_id, &shape_def, &box);
                }
            }
        }
    }
}

void Box2dPhysicsSystem::onDetach() {
    if (world_id_.is_none()) {
        return;
    }

    b2WorldId world_id = GetWorldId(world_id_.unwrap_unchecked());
    b2DestroyWorld(world_id);
    world_id_ = None();
}

}  // namespace cave
