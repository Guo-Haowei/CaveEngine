#include "UIRenderer.h"

namespace cave::render {

using math::Vector2f;
using math::Vector4f;

static void AppendUIRect(BuiltUIData& p_out,
                         const UIRect& p_rect,
                         const UIColor& p_color) {
    const float x0 = p_rect.Left();
    const float y0 = p_rect.Top();
    const float x1 = p_rect.Right();
    const float y1 = p_rect.Bottom();

    const Vector4f color = p_color.ToVector4f();

    const uint32_t base_vertex = static_cast<uint32_t>(p_out.vertices.size());

    p_out.vertices.push_back(UIVertex{
        .pos = Vector2f(x0, y0),
        .color = color,
    });

    p_out.vertices.push_back(UIVertex{
        .pos = Vector2f(x1, y0),
        .color = color,
    });

    p_out.vertices.push_back(UIVertex{
        .pos = Vector2f(x1, y1),
        .color = color,
    });

    p_out.vertices.push_back(UIVertex{
        .pos = Vector2f(x0, y1),
        .color = color,
    });

    p_out.indices.push_back(base_vertex + 0);
    p_out.indices.push_back(base_vertex + 1);
    p_out.indices.push_back(base_vertex + 2);

    p_out.indices.push_back(base_vertex + 0);
    p_out.indices.push_back(base_vertex + 2);
    p_out.indices.push_back(base_vertex + 3);
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

}  // namespace cave::render
