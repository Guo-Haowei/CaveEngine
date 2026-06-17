#pragma once
#include "cave/runtime/view/ViewDesc.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/ids/SceneId.h"

#include "cave/runtime/assets/AssetHandle.h"

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
    explicit PreviewBuilder(EngineServices& app) noexcept;
    ~PreviewBuilder();

    PreviewBuildResult build(const PreviewBuildRequest& req) const;

private:
    PreviewBuildResult buildMaterial(const AssetHandle& handle, const PreviewOptions& options) const;
    PreviewBuildResult buildMesh(const AssetHandle& handle, const PreviewOptions& options) const;
    PreviewBuildResult buildScene(const AssetHandle& handle, const PreviewOptions& options) const;

    AssetRegistry& asset_reg_;
    SceneRegistry& scene_reg_;
};

}  // namespace cave
