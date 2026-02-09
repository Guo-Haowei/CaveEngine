#include "RenderGraphViewer.h"

#include <imnodes/imnodes.h>

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/render/render_graph/CompiledGraph.h"
#include "cave/runtime/framework/IApplication.h"

#include "editor/EditorState.h"

namespace cave {

// @TODO: save the nodes position to disk
// @TODO: find longese path, and arrange nodes

RenderGraphViewer::RenderGraphViewer(EditorState& p_editor)
    : EditorWindow(p_editor) {
}

void RenderGraphViewer::DrawNodes(const render::CompiledGraph& p_graph) {
    using namespace render;

    std::span<const CompiledPass> passes = p_graph.GetCompiledPass();

    auto draw_node = [&passes, this](int id, float x, float y) {
        const CompiledPass& pass = passes[id];
        const bool flip_image = m_backend == Backend::OpenGL;

        ImNodes::BeginNode(id);

        if (m_firstFrame) {
            ImVec2 position(x, y);
            ImNodes::SetNodeGridSpacePos(id, position);
        }
        {
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(pass.name.c_str());
            ImNodes::EndNodeTitleBar();
        }
        ImGui::Spacing();
        {
            ImNodes::BeginInputAttribute(id << 16);
            ImGui::PushItemWidth(120.0f);
            ImGui::TextUnformatted("input");
            ImGui::PopItemWidth();
            ImNodes::EndInputAttribute();
        }

        auto add_image = [](bool p_flip, const std::shared_ptr<GpuTexture>& p_texture) {
            if (!p_texture) return;

            ImGui::Text("%s", p_texture->desc.name.c_str());
            if (p_texture && p_texture->desc.dimension == Dimension::TEXTURE_2D) {
                ImVec2 size(180 * 3, 120 * 3);
                if (p_flip) {
                    ImGui::Image(p_texture->GetHandle(), size, ImVec2(0, 1), ImVec2(1, 0));
                } else {
                    ImGui::Image(p_texture->GetHandle(), size);
                }
            }
        };

        ImGui::Spacing();
        {
            ImNodes::BeginStaticAttribute(id);
            for (const auto& srv : pass.srvs) {
                add_image(flip_image, srv);
            }
            ImNodes::EndStaticAttribute();
        }
        ImGui::Spacing();
        {
            ImNodes::BeginOutputAttribute(id << 24);

            const float text_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
            ImGui::TextUnformatted("output");

            ImNodes::EndOutputAttribute();
        }
        ImGui::Spacing();
        {
            ImNodes::BeginInputAttribute(id);

            for (const auto& rtv : pass.colors) {
                add_image(flip_image, rtv.tex);
            }
            ImNodes::EndInputAttribute();
        }

        ImNodes::EndNode();
    };

    const float initial_offset = 20.f;
    float x_offset = initial_offset;
    [[maybe_unused]] float y_offset = initial_offset;

    for (int i = 0; i < (int)passes.size(); ++i) {
        x_offset += 240.0f * 3;
        draw_node(i, x_offset, initial_offset);
    }

    // for (int from = 0; from < (int)adj_list.size(); ++from) {
    //     for (auto to : adj_list[from]) {
    //         const int id = (from << 24) | (to << 16);
    //         ImNodes::Link(id, from << 24, to << 16);
    //     }
    // }
}

void RenderGraphViewer::DrawUIImpl() {
    render::CompiledGraph* graph = nullptr;
    if (!graph) return;

    CAVE_PROFILE_EVENT();

    m_backend = m_editor.GetApp().GetBackend();

    switch (m_backend) {
        case Backend::Vulkan:
        case Backend::Metal:
            return;
        default:
            break;
    }

    ImNodes::BeginNodeEditor();

    DrawNodes(*graph);

    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
    ImNodes::EndNodeEditor();

    m_firstFrame = false;
}

}  // namespace cave
