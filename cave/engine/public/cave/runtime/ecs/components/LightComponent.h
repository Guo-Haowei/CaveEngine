// =============================================================================
// File: cave/runtime/ecs/components/LightComponent.h
// =============================================================================
#pragma once
#include "cave/core/math/AABB.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

// must be same order as "shader_defines.hlsl.h"
enum class LightType : uint8_t {
    Infinite = 0,
    Point,
    Spot,
    Area,
    Count,
};

DECLARE_ENUM_TRAITS(LightType, "infinite", "point", "spot", "area");

class LightComponent {
    CAVE_COMPONENT(LightComponent)

private:
    CAVE_PROP(editor = EnumDropDown)
    LightType m_type = LightType::Infinite;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_constant = 1.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_linear = 0.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_quadratic = 0.0f;

    CAVE_PROP()
    math::AABB m_shadow_region;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = false;

    // Non-serialized
    bool m_dirty = true;
    math::Vector3f m_position;
    float m_max_distance;
    std::array<math::Matrix4x4f, 6> m_light_space_matrices;

public:
    bool IsDirty() const { return m_dirty; }
    void SetDirty(bool p_dirty = true) { m_dirty = p_dirty; }

    bool CastShadow() const { return m_cast_shadow; }
    void SetCastShadow(bool p_cast_shadow = true) { m_cast_shadow = p_cast_shadow; }

    const math::AABB& GetShadowRegion() const { return m_shadow_region; }

    LightType GetType() const { return m_type; }
    void SetType(LightType p_type) { m_type = p_type; }

    void SetMaxDistance(float p_max_distance) { m_max_distance = p_max_distance; }
    float GetMaxDistance() const { return m_max_distance; }

    float GetAttenConstant() const { return m_atten_constant; }
    float GetAttenLinear() const { return m_atten_linear; }
    float GetAttenQuadratic() const { return m_atten_quadratic; }

    void SetPosition(const math::Vector3f& p_position) { m_position = p_position; }
    const math::Vector3f& GetPosition() const { return m_position; }

    auto& GetMatrices() { return m_light_space_matrices; }
    const auto& GetMatrices() const { return m_light_space_matrices; }

    void OnDeserialized();
};

}  // namespace cave
