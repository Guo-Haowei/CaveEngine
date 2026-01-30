#pragma once
#include "RenderScene.h"
#include "RenderSceneBuilder.h"

#include "cave/core/ids/SceneId.h"

// @TODO: remove
#include "engine/private/renderer/frame_data.h "

namespace cave {
struct FrameData;
class IApplication;
}  // namespace cave

namespace cave::render {

struct ViewDesc;

class RenderSystemImpl {
public:
    RenderSystemImpl(IApplication& p_app);

    void BeginFrame();

    void RenderFrame(std::span<const render::ViewDesc> p_views);

    std::span<const FrameData> GetFrameData() const { return m_frame_data; }

private:
    RenderScene& GetOrCreateRenderScene(SceneId p_scene_id);

private:
    IApplication& m_app;
    RenderSceneBuilder m_scene_builder;
    std::unordered_map<SceneId, RenderScene> m_scene_cache;

    std::vector<FrameData> m_frame_data;
};

}  // namespace cave::render
