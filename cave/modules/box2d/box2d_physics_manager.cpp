#include "box2d_physics_manager.h"

#include <box2d/box2d.h>

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
        // b2Rot rotation = b2Body_GetRotation(body_id);

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

void Box2dPhysicsManager::AddStaticBox(const b2WorldId& p_world_id, const Box2& p_box) {
    Vector2f center = p_box.Center();
    Vector2f half = 0.5f * p_box.Size();

    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.position = { center.x, center.y };
    body_def.fixedRotation = true;

    b2ShapeDef shape_def = b2DefaultShapeDef();
    body_def.type = b2_staticBody;

    b2BodyId body_id = b2CreateBody(p_world_id, &body_def);

    b2Polygon box = b2MakeBox(half.x, half.y);
    b2CreatePolygonShape(body_id, &shape_def, &box);
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
        const auto& boxes = tile_map_renderer.GetBoxes();
        Vector4f position = transform.GetWorldMatrix() * Vector4f::UnitW;
        Vector2f offset = position.xy;
        for (const Box2& _box : boxes) {
            Box2 box(_box.GetMin() + offset, _box.GetMax() + offset);
            AddStaticBox(world_id, box);
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
