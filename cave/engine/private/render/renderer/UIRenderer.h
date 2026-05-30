#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Vector.h"
#include "cave/runtime/framework/IUIRuntime.h"

#include "ResolvedView.h"

namespace cave::render {

struct UIVertex {
    math::Vector2f pos;
    math::Vector4f color;
};

struct UIBatch {
    ViewId view_id{};
    uint32_t first_index{};
    uint32_t index_count{};
};

struct BuiltUIData {
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<UIBatch> batches;
};

[[nodiscard]] BuiltUIData BuildUIData(std::span<const ResolvedView> p_views,
                                      const UIFrameDrawData& p_ui_data);

}  // namespace cave::render
