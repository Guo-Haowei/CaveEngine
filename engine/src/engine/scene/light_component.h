#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/aabb.h"
#include "engine/reflection/reflection.h"

#include "shader_defines.hlsl.h"

namespace cave {

class Archive;

class LightComponent {
    CAVE_META(LightComponent)

public:
    enum : uint32_t {
        NONE = BIT(0),
        DIRTY = BIT(1),
        CAST_SHADOW = BIT(2),
        SHADOW_REGION = BIT(3),
    };

    bool IsDirty() const { return m_flags & DIRTY; }
    void SetDirty(bool p_dirty = true) { p_dirty ? m_flags |= DIRTY : m_flags &= ~DIRTY; }

    bool CastShadow() const { return m_flags & CAST_SHADOW; }
    void SetCastShadow(bool p_cast = true) { p_cast ? m_flags |= CAST_SHADOW : m_flags &= ~CAST_SHADOW; }

    bool HasShadowRegion() const { return m_flags & SHADOW_REGION; }
    void SetShadowRegion(bool p_region = true) { p_region ? m_flags |= SHADOW_REGION : m_flags &= ~SHADOW_REGION; }

    int GetType() const { return m_type; }
    void SetType(int p_type) { m_type = p_type; }

    float GetMaxDistance() const { return m_maxDistance; }
    int GetShadowMapIndex() const { return m_shadowMapIndex; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized();

    const auto& GetMatrices() const { return m_lightSpaceMatrices; }
    const Vector3f& GetPosition() const { return m_position; }

    struct Attenuation {
        float constant;
        float linear;
        float quadratic;
    } m_atten;

    AABB m_shadowRegion;

    uint32_t m_flags = DIRTY;

    // @TODO: make light type enum
    CAVE_PROP(type = i32)
    int m_type = LIGHT_TYPE_INFINITE;

    CAVE_PROP(type = guid)
    Guid m_material_id;

    // Non-serialized
    float m_maxDistance;
    Vector3f m_position;
    int m_shadowMapIndex = -1;
    std::array<Matrix4x4f, 6> m_lightSpaceMatrices;
    Handle<MaterialAsset> m_material_handle;
};

}  // namespace cave
