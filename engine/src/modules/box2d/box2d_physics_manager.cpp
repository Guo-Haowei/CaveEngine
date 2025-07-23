#include "box2d_physics_manager.h"

#include <box2d/box2d.h>

namespace cave {

Result<void> Box2dPhysicsManager::InitializeImpl() {
    // setup
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, -10.0f };
    b2WorldId worldId = b2CreateWorld(&worldDef);
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = { 0.0f, -10.0f };

    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = { 0.0f, 4.0f };
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.3f;

    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
    // simulate

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;
    for (int i = 0; i < 90; ++i) {
        b2World_Step(worldId, timeStep, subStepCount);
        b2Vec2 position = b2Body_GetPosition(bodyId);
        b2Rot rotation = b2Body_GetRotation(bodyId);
        LOG_OK("x: {}, y: {}, rotation: {}", position.x, position.y, b2Rot_GetAngle(rotation));
    }

    b2DestroyWorld(worldId);

    return Result<void>();
}

void Box2dPhysicsManager::FinalizeImpl() {
    // @TODO: clean up
}

void Box2dPhysicsManager::Update(Scene& p_scene, float p_timestep) {
    unused(p_scene);
    unused(p_timestep);
}

void Box2dPhysicsManager::OnSimBegin(Scene& p_scene) {
    unused(p_scene);
}

void Box2dPhysicsManager::OnSimEnd() {
}

}  // namespace cave
