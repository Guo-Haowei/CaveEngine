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
    if (m_world_id.is_none()) {
        return;
    }

    const int sub_step_count = 4;

    b2WorldId world_id = GetWorldId(m_world_id.unwrap_unchecked());

    b2World_Step(world_id, p_timestep, sub_step_count);

    for (auto [id, collider] : p_scene.View<ColliderComponent>()) {
        TransformComponent* transform = p_scene.GetComponent<TransformComponent>(id);
        if (!transform) continue;
        b2BodyId body_id = std::bit_cast<b2BodyId>(collider.m_user_data);

        b2Vec2 position = b2Body_GetPosition(body_id);
        [[maybe_unused]] b2Rot rotation = b2Body_GetRotation(body_id);

        Vector3f translation = transform->GetTranslation();
        translation.x = position.x;
        translation.y = position.y;
        transform->SetTranslation(translation);
        transform->SetDirty();
    }
}

void Box2dPhysicsManager::OnSimBegin(Scene& p_scene) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, -10.0f };
    b2WorldId world_id = b2CreateWorld(&worldDef);

    m_world_id = Some(std::bit_cast<uint32_t>(world_id));

    for (auto [id, collider] : p_scene.View<ColliderComponent>()) {
        const TransformComponent* transform = p_scene.GetComponent<TransformComponent>(id);
        if (!transform) continue;

        Vector4f position = transform->GetWorldMatrix() * Vector4f::UnitW;
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
                shape_def.density = 1.0f;
                shape_def.material.friction = 0.3f;
            } break;
            case BodyType::Dynamic: {
                body_def.type = b2_dynamicBody;
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
