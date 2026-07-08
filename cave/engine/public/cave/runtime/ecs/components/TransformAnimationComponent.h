// =============================================================================
// File: cave/runtime/ecs/components/TransformAnimationComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/Vec.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/intent/Intent.h"

namespace cave {

struct TransformAnimationFinishedEvent : public Intent {
    CAVE_DECLARE_INTENT("trans.anim.finished");

    TransformAnimationFinishedEvent(ecs::Entity p_ent)
        : ent(p_ent) {}

    const ecs::Entity ent;
};

// @TODO: figure out the generic part
// of TransformAnimationComponent, SpriteAnimationClip
// and SkeletalAnimationComponent
class TransformAnimationComponent {
    CAVE_COMPONENT(TransformAnimationComponent)

public:
    CAVE_PROP(editor = Translation)
    math::Vec3f begin;

    CAVE_PROP(editor = Translation)
    math::Vec3f end;

    CAVE_PROP(editor = InputFloat)
    float duration = 0.18f;

    CAVE_PROP(editor = DragFloat)
    float elapsed = 0.0f;

    CAVE_PROP(editor = Toggle)
    bool playing = false;

    CAVE_PROP(editor = Toggle)
    bool destroy_on_finish = false;

    void OnDeserialized() {}
};

}  // namespace cave
