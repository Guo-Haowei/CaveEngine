#pragma once
#include "RenderScene.h"
#include "RenderSceneBuilder.h"

#include "cave/core/ids/SceneId.h"

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

    const FrameData* GetFrameData() const { return m_frameData; }

private:
    RenderScene& GetOrCreateRenderScene(SceneId p_scene_id);

private:
    IApplication& m_app;
    RenderSceneBuilder m_scene_builder;
    std::unordered_map<SceneId, RenderScene> m_scene_cache;

    // @TODO: deprecate
    FrameData* m_frameData{ nullptr };
};

}  // namespace cave::render
