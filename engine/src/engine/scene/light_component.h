#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/aabb.h"
#include "engine/reflection/reflection.h"

#include "shader_defines.hlsl.h"

namespace cave {

class LightComponent {
    CAVE_META(LightComponent)

    CAVE_PROP()
    LightType m_type = LIGHT_TYPE_INFINITE;

    CAVE_PROP(editor = Color)
    Vector4f m_base_color = Vector4f::One;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float m_emissive = 4.0f;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_constant;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_linear;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1)
    float m_atten_quadratic;

    CAVE_PROP()
    AABB m_shadow_region;

    CAVE_PROP(editor = Toggle)
    bool m_cast_shadow = false;

    // Non-serialized
    bool m_dirty = true;
    Vector3f m_position;
    float m_max_distance;
    std::array<Matrix4x4f, 6> m_light_space_matrices;

public:
    bool IsDirty() const { return m_dirty; }
    void SetDirty(bool p_dirty = true) { m_dirty = p_dirty; }

    bool CastShadow() const { return m_cast_shadow; }
    void SetCastShadow(bool p_cast_shadow = true) { m_cast_shadow = p_cast_shadow; }

    const AABB& GetShadowRegion() const { return m_shadow_region; }

    int GetType() const { return m_type; }
    void SetType(LightType p_type) { m_type = p_type; }

    void SetMaxDistance(float p_max_distance) { m_max_distance = p_max_distance; }
    float GetMaxDistance() const { return m_max_distance; }

    float GetAttenConstant() const { return m_atten_constant; }
    float GetAttenLinear() const { return m_atten_linear; }
    float GetAttenQuadratic() const { return m_atten_quadratic; }

    void SetPosition(const Vector3f& p_position) { m_position = p_position; }
    const Vector3f& GetPosition() const { return m_position; }

    auto& GetMatrices() { return m_light_space_matrices; }
    const auto& GetMatrices() const { return m_light_space_matrices; }

    const Vector4f& GetBaseColor() const { return m_base_color; }
    float GetEmissive() const { return m_emissive; }

    void OnDeserialized();

    friend class EntityFactory;
};

}  // namespace cave
