#pragma once
#include <memory>

#include "cave/runtime/display/IDebugDrawService.h"

namespace cave {

class DebugDrawService : public IDebugDrawService {

public:
    void addBox2Frame(const math::Vec2f& min,
                      const math::Vec2f& max,
                      const math::Vec4f& color,
                      const math::Mat4f* transform,
                      float thickness) override;

    void addBox2(const math::Vec2f& min,
                 const math::Vec2f& max,
                 const math::Vec4f& color,
                 const math::Mat4f* transform) override;

    std::span<const DebugDrawItem> items() const override {
        return items_;
    }

private:
    std::vector<DebugDrawItem> items_;
};

}  // namespace cave
