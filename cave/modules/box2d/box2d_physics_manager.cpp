#include "box2d_physics_manager.h"

#include <box2d/box2d.h>

#include "engine/assets/tile_map_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/scene/scene.h"

namespace cave {

static_assert(sizeof(b2WorldId) == sizeof(uint32_t));
static_assert(sizeof(b2BodyId) == sizeof(uint64_t));

Result<void> Box2dPhysicsManager::InitializeImpl() {
    return Result<void>();
}

void Box2dPhysicsManager::FinalizeImpl() {
    OnSimEnd();
}

static b2WorldId GetWorldId(uint32_t p_raw_id) {
    return std::bit_cast<b2WorldId>(p_raw_id);
}

void Box2dPhysicsManager::Update(Scene& p_scene, float p_timestep) {
    constexpr int sub_step_count = 4;

    if (m_world_id.is_none()) {
        return;
    }

    b2WorldId world_id = GetWorldId(m_world_id.unwrap_unchecked());

    // 1. set speed
    {
        auto view = p_scene.View<ColliderComponent, VelocityComponent>();
        for (auto [id, collider, vel] : view) {
            b2BodyId body_id = std::bit_cast<b2BodyId>(collider.m_user_data);
            b2Body_SetLinearVelocity(body_id, { vel.linear.x, vel.linear.y });
        }
    }

    // 2. simulate
    b2World_Step(world_id, p_timestep, sub_step_count);

    // 3. sync speed and position
    auto view = p_scene.View<ColliderComponent, TransformComponent>();
    for (auto [id, collider, transform] : view) {
        b2BodyId body_id = std::bit_cast<b2BodyId>(collider.m_user_data);

        b2Vec2 position = b2Body_GetPosition(body_id);
        [[maybe_unused]] b2Rot rotation = b2Body_GetRotation(body_id);

        Vector3f translation = transform.GetTranslation();
        translation.x = position.x;
        translation.y = position.y;
        transform.SetTranslation(translation);
        transform.SetDirty();

        if (VelocityComponent* vel = p_scene.GetComponent<VelocityComponent>(id); vel) {
            b2Vec2 linear = b2Body_GetLinearVelocity(body_id);
            vel->linear.x = linear.x;
            vel->linear.y = linear.y;
        }
    }
}

void Box2dPhysicsManager::OnSimBegin(Scene& p_scene) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, -20.0f };
    b2WorldId world_id = b2CreateWorld(&worldDef);

    m_world_id = Some(std::bit_cast<uint32_t>(world_id));

    for (auto [id, collider, transform] : p_scene.View<ColliderComponent, TransformComponent>()) {
        Vector4f position = transform.GetWorldMatrix() * Vector4f::UnitW;
        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.position = { position.x, position.y };
        body_def.fixedRotation = true;
#if USING(DEBUG_BUILD)
        const NameComponent* name = p_scene.GetComponent<NameComponent>(id);
        if (name) {
            body_def.name = name->GetName().c_str();
        }
#endif

        b2ShapeDef shape_def = b2DefaultShapeDef();

        switch (collider.GetBodyType()) {
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

        const Shape& shape = collider.GetShape();
        switch (shape.type) {
            case ShapeType::Box: {
                const auto& half = shape.data.half;
                b2Polygon box = b2MakeBox(half.x, half.y);
                b2CreatePolygonShape(body_id, &shape_def, &box);

                collider.m_user_data = std::bit_cast<size_t>(body_id);
            } break;
            default:
                break;
        }
    }

    for (auto [id, tile_map_renderer, transform] : p_scene.View<TileMapRendererComponent, TransformComponent>()) {
        const TileMapAsset* tile_map = tile_map_renderer.GetTileMapHandle().Get();
        if (!tile_map) continue;
        const TileSetAsset* tile_set = tile_map->GetTileSetHandle().Get();
        if (!tile_set) continue;

        Vector4f position = transform.GetWorldMatrix() * Vector4f::UnitW;

        const auto& colliders = tile_set->GetColliders();
        const auto& chunks = tile_map->GetTiles().chunks;
        for (const auto& [key, chunk_ptr] : chunks) {
            const int16_t offset_x = key.x * TILE_CHUNK_SIZE;
            const int16_t offset_y = key.y * TILE_CHUNK_SIZE;
            const auto& chunk = chunk_ptr->tiles;
            for (int16_t y = offset_y; y < offset_y + TILE_CHUNK_SIZE; ++y) {
                for (int16_t x = offset_x; x < offset_x + TILE_CHUNK_SIZE; ++x) {
                    const TileId& tile_id = chunk[y - offset_y][x - offset_x];
                    auto it = colliders.find(tile_id);
                    if (it == colliders.end()) continue;
                    const Shape& shape = it->second;
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

void Box2dPhysicsManager::OnSimEnd() {
    if (m_world_id.is_none()) {
        return;
    }

    b2WorldId world_id = GetWorldId(m_world_id.unwrap_unchecked());
    b2DestroyWorld(world_id);
    m_world_id = None();
}

}  // namespace cave
