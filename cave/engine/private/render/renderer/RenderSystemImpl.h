#pragma once

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

    void RenderFrame(const std::vector<ViewDesc>& p_views);

    const FrameData* GetFrameData() const { return m_frameData; }

protected:
    IApplication& m_app;
    FrameData* m_frameData{ nullptr };
};

}  // namespace cave::render
