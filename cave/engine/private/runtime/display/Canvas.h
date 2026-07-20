#pragma once
#include "cave/runtime/view/ViewRecord.h"
#include "cave/runtime/display/ICanvas.h"

namespace cave {

class Canvas : public ICanvas {
    static const int kInvalidIdx = -1;

public:
    void beginFrame() override;
    void endFrame() override;

    void pushView(ViewId view_id) override;
    void popView() override;

    void addBox2Frame(const math::Vec2f& min,
                      const math::Vec2f& max,
                      float thickness,
                      const Draw2DOptions& options) override;

    void addBox2(const math::Vec2f& min,
                 const math::Vec2f& max,
                 const Draw2DOptions& options) override;

    void addImage(const GpuTexture* texture,
                  const math::Vec2f& min,
                  const math::Vec2f& max,
                  const ImageDrawOptions& options) override;

    bool takeBucket(ViewId view_id, CanvasBucket& out) override;

private:
    void addImageImpl(const GpuTexture* texture,
                      const math::Vec2f& min,
                      const math::Vec2f& max,
                      const ImageDrawOptions& options);

    bool canSubmit() const;

    bool m_can_submit = false;
    int m_current_idx = kInvalidIdx;
    Vector<ViewId> m_stack;  // @TODO: small vector?

    Vector<CanvasBucket> m_buckets;
    HashMap<ViewId, int> m_lookup;
};

}  // namespace cave
