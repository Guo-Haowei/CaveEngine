#pragma once
#include "cave/render/ViewDesc.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/ids/SceneId.h"

#include "engine/private/assets/asset_handle.h"

namespace cave {

class AssetRegistry;
class IApplication;
class ISceneRegistry;

struct PreviewOptions {
    uint32_t width = 256;
    uint32_t height = 256;
    float fov_y_deg = 50.0f;

    math::Vector3f view_dir{ 0, 0, -1 };
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
    render::CameraSource camera{};
};

class PreviewBuilder {
public:
    explicit PreviewBuilder(IApplication& p_app) noexcept;
    ~PreviewBuilder();

    PreviewBuildResult Build(const PreviewBuildRequest& p_req) const;

private:
    PreviewBuildResult BuildMaterial(const AssetHandle& p_handle, const PreviewOptions& p_options) const;
    PreviewBuildResult BuildMesh(const AssetHandle& p_handle, const PreviewOptions& p_options) const;
    PreviewBuildResult BuildScene(const AssetHandle& p_handle, const PreviewOptions& p_options) const;

    AssetRegistry& m_asset_reg;
    ISceneRegistry& m_scene_reg;
};

}  // namespace cave
