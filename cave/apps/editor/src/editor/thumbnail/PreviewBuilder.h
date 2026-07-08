#pragma once
#include "cave/runtime/view/ViewDesc.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/ids/SceneId.h"

#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/scene/SceneTickContext.h"

namespace cave {

struct EngineServices;
class AssetRegistry;
class SceneRegistry;

struct PreviewOptions {
    uint32_t width = 256;
    uint32_t height = 256;
    float fov_y_deg = 50.0f;
};

struct PreviewBuildRequest {
    Guid guid;
    PreviewOptions options{};
};

enum class PreviewBuildStatus : uint8_t {
    Ok,
    Error,
};

struct PreviewBuildResult {
    PreviewBuildStatus status{};
    SceneId scene_id{};
    CameraSource camera{};
};

class PreviewBuilder {
public:
    explicit PreviewBuilder(EngineServices& engine_services) noexcept;
    ~PreviewBuilder();

    PreviewBuildResult build(const PreviewBuildRequest& req) const;

private:
    PreviewBuildResult buildMaterial(const AssetMetaData* meta,
                                     const AssetHandle& handle,
                                     const PreviewOptions& options) const;

    PreviewBuildResult buildMesh(const AssetMetaData* meta,
                                 const AssetHandle& handle,
                                 const PreviewOptions& options) const;

    PreviewBuildResult buildSceneImpl(const AssetMetaData* meta,
                                      const Scene& scene,
                                      const PreviewOptions& options) const;

    SceneTickContext makeSceneContext(Scene& scene) const;

    AssetRegistry& m_asset_reg;
    SceneRegistry& m_scene_reg;
    EngineServices& m_engine_services;
};

}  // namespace cave
