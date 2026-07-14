#include "UIRenderer.h"

namespace cave::render {

#if 0
using math::Vec2f;
using math::Vec4f;

static void AppendUIRect(BuiltUIData& out,
                         const UIRect& rect,
                         const Color& color) {
    const float x0 = rect.min().x;
    const float y0 = rect.min().y;
    const float x1 = rect.max().x;
    const float y1 = rect.max().y;

    const uint32_t base_vertex = static_cast<uint32_t>(out.positions.size());

    out.positions.push_back(rect.min());
    out.positions.push_back(Vec2f(x1, y0));
    out.positions.push_back(rect.max());
    out.positions.push_back(Vec2f(x0, y1));

    out.colors.push_back(color);
    out.colors.push_back(color);
    out.colors.push_back(color);
    out.colors.push_back(color);

    out.indices.push_back(base_vertex + 0);
    out.indices.push_back(base_vertex + 1);
    out.indices.push_back(base_vertex + 2);

    out.indices.push_back(base_vertex + 0);
    out.indices.push_back(base_vertex + 2);
    out.indices.push_back(base_vertex + 3);
}

BuiltUIData BuildUIData(std::span<const ResolvedView> p_views,
                        const UIFrameDrawData& p_ui_data) {
    BuiltUIData out;
    out.batches.resize(p_views.size());

    int i = 0;
    for (const ResolvedView& view : p_views) {
        const ViewId view_id = view.view_id;

        UIBatch& batch = out.batches[i++];
        batch.view_id = view_id;
        batch.first_index = static_cast<uint32_t>(out.indices.size());
        batch.index_count = 0;

        auto it = p_ui_data.draw_lists.find(view_id);
        if (it == p_ui_data.draw_lists.end()) {
            continue;
        }

        const UIDrawList& list = it->second;
        if (list.cmds.empty()) {
            continue;
        }

        for (const UIDrawCommand& cmd : list.cmds) {
            switch (cmd.type) {
                case UIDrawCommandType::Rect: {
                    AppendUIRect(out, cmd.rect.rect, cmd.rect.color);
                } break;
                default: {
                    DEV_ASSERT(0 && "NOT IMPLEMENTED");
                } break;
            }
        }

        batch.index_count =
            static_cast<uint32_t>(out.indices.size()) - batch.first_index;
    }

    return out;
}
#endif

}  // namespace cave::render
