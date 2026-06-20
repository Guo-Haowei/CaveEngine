#pragma once
#include <memory>

#include "cave/runtime/display/IDebugDrawService.h"

namespace cave {

class DebugDrawService : public IDebugDrawService {

public:
    void addBox2Frame(const math::Vec2f& min,
                      const math::Vec2f& max,
                      const math::Vec4f& color,
                      float thickness,
                      const math::Mat4f* transform) override;

    void addBox2(const math::Vec2f& min,
                 const math::Vec2f& max,
                 const math::Vec4f& color,
                 const math::Mat4f* transform) override;

    auto items() const -> std::span<const DebugDrawItem> override {
        return items_;
    }

    void clear() override { items_.clear(); }

private:
    std::vector<DebugDrawItem> items_;
};

}  // namespace cave
