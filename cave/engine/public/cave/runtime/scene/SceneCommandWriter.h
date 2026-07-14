// =============================================================================
// File: cave/runtime/scene/SceneCommandWriter.h
// =============================================================================
#pragma once
#include <string_view>

#include "cave/core/math/Vec.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/scene/SceneCommandBuffer.h"

namespace cave {

class AssetRegistry;

struct MaterialContext {
    const Guid* guid{ nullptr };
    math::Vec4f base_color{ 1 };
};

class SceneCommandWriter : public SceneCommandBuffer {
    using Entity = ecs::Entity;
    using Vec3f = math::Vec3f;

public:
    explicit SceneCommandWriter(AssetRegistry& reg) noexcept
        : m_asset_reg(reg) {
    }

    void attachChild(Entity child, Entity parent);

    Entity rootObject(std::string_view name = "root");
    Entity nameObject(std::string_view name);

    Entity prefabObject(std::string_view name, const Guid& guid = Guid::null());

    Entity transformObject(std::string_view name);

    Entity pointLightObject(std::string_view name,
                            const Vec3f& position = Vec3f(0.0f, 1.0f, 0.0f),
                            const Vec3f& color = Vec3f(1.0f),
                            float emissive = 5.0f);

    Entity areaLightObject(std::string_view name,
                           const Vec3f& color = Vec3f(1),
                           float emissive = 5.0f);

    Entity infiniteLightObject(std::string_view name,
                               const Vec3f& color = Vec3f(1),
                               float emissive = 5.0f);

    Entity meshObject(const std::string& mesh_path,
                      std::string_view name,
                      const MaterialContext& mat_ctx);

    Entity meshObject(const std::string& mesh_path,
                      std::string_view name,
                      const std::string& mat_path);

    Entity planeObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/plane", name, mat_ctx);
    }

    Entity cubeObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/cube", name, mat_ctx);
    }

    Entity sphereObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/sphere", name, mat_ctx);
    }

    Entity cylinderObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/cylinder", name, mat_ctx);
    }

    Entity coneObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/cone", name, mat_ctx);
    }

    Entity torusObject(std::string_view name, const MaterialContext& mat_ctx = {}) {
        return meshObject("@persist://meshes/torus", name, mat_ctx);
    }

    Entity tileMapObject(std::string_view name);

    Entity canvas(std::string_view name);
    Entity rect(std::string_view name);
    Entity button(std::string_view name);
    Entity image(std::string_view name);

    void setNoSave(bool value) { m_no_save = value; }
    bool noSave() const { return m_no_save; }

private:
    AssetRegistry& m_asset_reg;
    bool m_no_save{ false };
};

}  // namespace cave
