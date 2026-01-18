#pragma once

namespace cave {

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    // virtual void Tick(float dt, const FViewportInput& input, bool bFocused) = 0;

    // 1..N views
    // virtual void BuildViews(const FIntRect& viewRect,
    //                        FRenderTarget* rt,
    //                        std::vector<FRenderView>& outViews) = 0;

    virtual const char* GetDebugName() const = 0;
};

}  // namespace cave