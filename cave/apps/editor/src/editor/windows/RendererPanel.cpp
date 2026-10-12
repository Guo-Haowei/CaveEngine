#include "RendererPanel.h"

#include <imgui/imgui_internal.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/features/path_tracer/PathTracerFeature.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/runtime/framework/CommonDvars.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/scene/Scene.h"

#include "editor/EditorDvars.h"
#include "editor/EditorState.h"

#include "engine/private/render/renderer/Renderer.h"
// @TODO: remove
#include "engine/private/renderer/graphics_dvars.h"

namespace cave {

RendererPanel::RendererPanel(EditorState& editor)
    : EditorWindow(editor)
    , m_renderer(m_engine_services.renderer()) {}

static void CollapseWindow(const std::string& window_name,
                           std::function<void(void)>&& func,
                           bool disabled = false) {
    const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                     ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
                                     ImGuiTreeNodeFlags_FramePadding;
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx(window_name.c_str(), flags, "%s", window_name.c_str());
        ImGui::PopStyleVar();

        if (open) {
            if (disabled) ImGui::BeginDisabled();
            func();
            ImGui::TreePop();
            if (disabled) ImGui::EndDisabled();
        }
    }
}

void RendererPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    const render::RendererCapabilities& capabilities = m_renderer.getCapabilities();

    ImGui::TextUnformatted("Debug");
    ImGui::Text("Frame rate:%.2f", ImGui::GetIO().Framerate);
    ImGui::Checkbox("show editor", (bool*)DVAR_GET_POINTER(show_editor));
    ImGui::Checkbox("debug UI", (bool*)DVAR_GET_POINTER(r_debug_ui));

    CollapseWindow("Texture Pool", [this]() {
        auto snapshot = m_renderer.poolSnapshot();
        if (snapshot.textures.empty()) {
            ImGui::TextUnformatted("No textures in pool.");
            return;
        }

        constexpr float preview_h = 128.0f;

        constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("##TexturePoolTable", 2, table_flags, ImVec2(0.0f, 400.0f))) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 64.0f);
            ImGui::TableSetupColumn("Texture", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();

            for (const auto& tex : snapshot.textures) {
                // Expand row height to fit the image
                ImGui::TableNextRow(ImGuiTableRowFlags_None, preview_h + 8.0f);

                // 1. Debug Name & Dimensions
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%.*s", static_cast<int>(tex.debug_name.size()), tex.debug_name.data());
                ImGui::TextDisabled("%ux%u | %u Mips", tex.width, tex.height, tex.mips);

                // 2. Render Texture Preview
                ImGui::TableSetColumnIndex(1);
                ImTextureID img_id = tex.texture->GetHandle();

                float aspect = (tex.height > 0) ? static_cast<float>(tex.width) / static_cast<float>(tex.height) : 1.0f;
                ImVec2 img_size(preview_h * aspect, preview_h);

                m_engine_services.imgui->image(img_id, img_size);

                // Hover popup for large inspection view
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%.*s (%ux%u)", static_cast<int>(tex.debug_name.size()), tex.debug_name.data(), tex.width, tex.height);
                    constexpr float tooltip_h = 320.0f;
                    m_engine_services.imgui->image(img_id, ImVec2(tooltip_h * aspect, tooltip_h));

                    ImGui::EndTooltip();
                }
            }

            ImGui::EndTable();
        }
    });

    CollapseWindow("Shadow", []() {
        ImGui::Checkbox("debug", (bool*)DVAR_GET_POINTER(gfx_debug_shadow));
    });

    CollapseWindow("IBL", []() { ImGui::Checkbox("enable", (bool*)DVAR_GET_POINTER(gfx_enable_ibl)); }, !capabilities.canRunIBL);

    CollapseWindow("Bloom", []() {
        ImGui::Checkbox("enable", (bool*)DVAR_GET_POINTER(gfx_enable_bloom));
        ImGui::DragFloat("threshold", (float*)DVAR_GET_POINTER(gfx_bloom_threshold), 0.01f, 0.0f, 3.0f); }, !capabilities.canRunBloom);

    CollapseWindow("SSAO", []() {
        ImGui::Checkbox("enable", (bool*)DVAR_GET_POINTER(gfx_ssao_enabled));
        ImGui::DragFloat("kernel radius", (float*)DVAR_GET_POINTER(gfx_ssao_radius), 0.01f, 0.0f, 5.0f);
    });

    CollapseWindow("Path Tracer", [&]() {
        using namespace render;
        bool active = IsPathTracerActive();
        if (ImGui::Checkbox("start", &active)) {
            if (active) {
                SetPathTracerMode(PathTracerMode::INTERACTIVE);
            } else {
                SetPathTracerMode(PathTracerMode::NONE);
            }
        }

        if (ImGui::Button("Generate BVH")) {
            DVAR_SET_BOOL(gfx_bvh_generate, true);
        }
        int bvh_level = DVAR_GET_INT(gfx_bvh_debug);
        // drawing too many bvh nodes causes performance issue,
        // set a maximum number
        if (ImGui::DragInt("Debug BVH", &bvh_level, 0.1f, -1, 10)) {
            DVAR_SET_INT(gfx_bvh_debug, bvh_level);
        }
    });
}

}  // namespace cave
