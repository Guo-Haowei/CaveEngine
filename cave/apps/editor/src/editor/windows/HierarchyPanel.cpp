#include "HierarchyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui_internal.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/edit/EditObjectCmd.h"
#include "editor/prefab/PrefabExporter.h"
#include "editor/services/DocumentService.h"
#include "editor/services/DragDropService.h"
#include "editor/services/EditService.h"
#include "editor/services/EditorServices.h"
#include "editor/services/SceneEditService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

// @TODO: fix
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

namespace cave {

using ::cave::ecs::Entity;

namespace {

constexpr char kPopupNameId[] = "SCENE_PANEL_POPUP";

// @TODO: build the scene tree and attach to scene
// @TODO: on scene change instead of build every frame
class SceneTreeBuilder {
public:
    SceneTreeBuilder(const SceneEditContext& context,
                     Scene& scene,
                     EngineServices& engine_services,
                     EditorServices& editor_services)
        : m_context(context)
        , m_scene(scene)
        , m_engine_services(engine_services)
        , m_editor_services(editor_services) {}

    void draw() {
        for (Entity root : m_scene.hierarchy().roots()) {
            drawNode(root, 0);
        }
    }

private:
    void drawNode(Entity node,
                  ImGuiTreeNodeFlags flags = 0);
    bool treeNodeHelper(Scene& scene,
                        Entity ent,
                        ImGuiTreeNodeFlags tree_flags,
                        std::function<void()> on_left_click,
                        std::function<void()> on_right_click);
    const SceneEditContext& m_context;

    Scene& m_scene;
    EngineServices& m_engine_services;
    EditorServices& m_editor_services;
};

bool SceneTreeBuilder::treeNodeHelper(Scene& scene,
                                      Entity ent,
                                      ImGuiTreeNodeFlags tree_flags,
                                      std::function<void()> on_left_click,
                                      std::function<void()> on_right_click) {
    const auto* name_component = scene.component<NameComponent>(ent);
    const auto* hier_component = scene.component<HierarchyComponent>(ent);
    if (!DEV_VERIFY(name_component && hier_component)) {
        return false;
    }

    std::string_view name = name_component->name();
    if (name.empty()) {
        name = "Untitled";
    }

    const char* icon = ICON_FA_FOLDER;
    if (tree_flags & ImGuiTreeNodeFlags_Leaf) {
        icon = ICON_FA_SQUARE_SHARE_NODES;
    }
    if (scene.storage().has(PrefabInstanceComponent_Id, ent)) {
        icon = ICON_FA_CUBE;
    }

    const char* text = hier_component->visible() ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
    const auto node_name = std::format("##tree_node_{}", ent.id());
    const auto selectable_name = std::format("{} {}##tree_selectable_{}", icon, name, ent.id());
    const auto visibility_name = std::format("{}##visibility_{}", text, ent.id());

    tree_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;

    constexpr float kVisibilityColumnWidth = 36.0f;
    constexpr float kRightPadding = 4.0f;

    ImGui::PushID(static_cast<int>(ent.id()));

    const float row_start_y = ImGui::GetCursorPosY();

    const bool expanded = ImGui::TreeNodeEx(node_name.c_str(), tree_flags);

    ImGui::SameLine();

    const float available_width = ImGui::GetContentRegionAvail().x;
    float selectable_width = available_width -
                             kVisibilityColumnWidth -
                             kRightPadding -
                             ImGui::GetStyle().ItemSpacing.x;
    selectable_width = math::max(selectable_width, 1.0f);

    ImGui::Selectable(selectable_name.c_str(),
                      false,
                      ImGuiSelectableFlags_None,
                      ImVec2(selectable_width, 0.0f));

    const bool selectable_hovered = ImGui::IsItemHovered();

    bool left_clicked = false;
    bool right_clicked = false;

    if (selectable_hovered) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            left_clicked = true;
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            right_clicked = true;
        }
    }

    m_editor_services.dragDrop().dragSceneNode(ent, name);
    m_editor_services.dragDrop().dropSceneNode(ent, m_context.doc_id, scene);

    const float visibility_x = ImGui::GetWindowContentRegionMax().x -
                               kVisibilityColumnWidth -
                               kRightPadding;

    ImGui::SameLine();
    ImGui::SetCursorPosX(visibility_x);
    ImGui::SetCursorPosY(row_start_y);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 2.0f));

    if (ImGui::Button(visibility_name.c_str(), ImVec2(kVisibilityColumnWidth, 0.0f))) {
        auto cmd = MakeOwner<ChangePropertyCmd>(
            m_engine_services.sceneRegistry(),
            ComponentPropertyTarget{ ent, HierarchyComponent_Id, CAVE_SID("local_visible") },
            hier_component->localVisible(),
            !hier_component->localVisible());

        m_editor_services.edit().submit(m_context.doc_id, std::move(cmd));
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::PopID();

    if (left_clicked && on_left_click) {
        on_left_click();
    }
    if (right_clicked && on_right_click) {
        on_right_click();
    }

    return expanded;
}

}  // namespace

// @TODO: make it an widget
void SceneTreeBuilder::drawNode(Entity ent, ImGuiTreeNodeFlags tree_flags) {
    DEV_ASSERT(ent.valid());

    Entity current_id = ent;

    SelectionKey selection = m_editor_services.selection().primary(m_context.doc_id);

    auto children = m_scene.hierarchy().children(ent);

    tree_flags |= children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
    tree_flags |= current_id == selection.entity ? ImGuiTreeNodeFlags_Selected : 0;

    const bool expanded = treeNodeHelper(
        m_scene,
        current_id,
        tree_flags,
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_context.doc_id;
            selection.scene = m_context.scene_id;
            selection.entity = current_id;

            m_editor_services.selection().setSelection(m_context.doc_id, selection);
        },
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_context.doc_id;
            selection.scene = m_context.scene_id;
            selection.entity = current_id;

            m_editor_services.selection().setSelection(m_context.doc_id, selection);
            ImGui::OpenPopup(kPopupNameId);
        });

    if (expanded) {
        float indentWidth = 8.f;
        ImGui::Indent(indentWidth);

        for (auto& child : children) {
            drawNode(child);
        }
        ImGui::Unindent(indentWidth);
    }
}

void HierarchyPanel::drawToolbar(const SceneEditContext* context,
                                 const Scene* scene) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float button_width = ImGui::GetFrameHeight();
    const float toolbar_width = button_width * 3.5f + style.ItemSpacing.x;

    const float avail_width = ImGui::GetContentRegionAvail().x;
    const float cursor_x = ImGui::GetCursorPosX();

    ImGui::SetCursorPosX(cursor_x + std::max(0.0f, avail_width - toolbar_width));

    ImGui::BeginGroup();

    if (ImGui::Button(ICON_FA_PLUS, ImVec2{ button_width, 0.0f })) {
        unused(context);
        unused(scene);
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("Add a new object");
    }

    ImGui::SameLine();

    // ImGui::BeginDisabled(!m_selected);
    bool m_selected = true;

    if (ImGui::Button(ICON_FA_TRASH_CAN, ImVec2{ button_width, 0.0f })) {
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(m_selected ? "Delete selected object" : "Select an object first");
    }

    // ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ELLIPSIS, ImVec2{ button_width, 0.0f })) {
    }

    ImGui::EndGroup();
}

void HierarchyPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();

    const float footer_size = ImGui::GetFrameHeight() + 10.f;
    ImGui::BeginChild("##SceneTree",
                      ImVec2{ 0.0f, -footer_size },
                      ImGuiChildFlags_Borders);

    SceneEditContext* ctx = nullptr;
    Scene* scene = nullptr;
    if ((ctx = m_editor_services.sceneEdit().current()) != nullptr) {
        if ((scene = m_engine_services.sceneRegistry().resolve(ctx->scene_id)) != nullptr) {
            SceneTreeBuilder sceneTree(*ctx, *scene, m_engine_services, m_editor_services);
            drawPopup(*ctx, *scene);
            sceneTree.draw();
        }
    }

    ImGui::EndChild();

    drawToolbar(ctx, scene);
}

void HierarchyPanel::drawPopup(const SceneEditContext& context,
                               const Scene& scene) {
    if (ImGui::BeginPopup(kPopupNameId)) {
        SelectionKey selection = m_editor_services.selection().primary(context.doc_id);
        DEV_ASSERT(selection.doc == context.doc_id);
        ecs::Entity selected = selection.entity;

        ecs::Entity parent = selected;

        if (ImGui::BeginMenu("Add")) {
            const bool is_ui = scene.has(UICanvasComponent_Id, parent) ||
                               scene.has(UIRectTransformComponent_Id, parent);
            if (is_ui) {
                openAddUIPopupImpl(context, scene, parent);
            } else {
                openAddEntityPopupImpl(context, scene, parent);
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Copy")) {
            // Clipboard service
            LOG_WARN("TODO: fix Copy");
        }
        if (ImGui::MenuItem("Paste")) {
            LOG_WARN("TODO: fix Paste");
        }
        if (ImGui::MenuItem("Delete")) {
            if (selected.valid()) {
                auto cmd = MakeOwner<DeleteObjectCmd>(
                    m_engine_services.sceneRegistry(),
                    selected);
                m_editor_services.edit().submit(context.doc_id, std::move(cmd));
            }
        }
        if (ImGui::MenuItem("Save as Prefab")) {
            PrefabExporter exporter;
            exporter.exportPrefab("@res://exported.prefab", scene, selected);
        }
        ImGui::EndPopup();
    }
}

// clang-format off
#define OBJECT_LIST                      \
    DEFINE_OBJECT(transform,      true)  \
    DEFINE_OBJECT(prefab,         true)  \
    DEFINE_OBJECT(infiniteLight,  false) \
    DEFINE_OBJECT(pointLight,     false) \
    DEFINE_OBJECT(areaLight,      true ) \
    DEFINE_OBJECT(plane,          false) \
    DEFINE_OBJECT(cube,           false) \
    DEFINE_OBJECT(sphere,         false) \
    DEFINE_OBJECT(cylinder,       false) \
    DEFINE_OBJECT(cone,           false) \
    DEFINE_OBJECT(torus,          true )
// clang-format on

void HierarchyPanel::openAddUIPopupImpl(const SceneEditContext& context,
                                        const Scene& scene,
                                        Entity parent) {
    EditService& edit = m_editor_services.edit();

    auto can_add_canvas_item = [](const Scene& scene, Entity parent) {
        while (parent.valid()) {
            if (scene.has(UICanvasComponent_Id, parent)) return true;
            const auto* hier = scene.component<HierarchyComponent>(parent);
            if (DEV_VERIFY(hier)) {
                parent = hier->parent();
            } else {
                return false;
            }
        }
        return false;
    };

    const bool can_add_ui = can_add_canvas_item(scene, parent);

    if (ImGui::MenuItem("UIRect", nullptr, false, can_add_ui)) {
        edit.submit(context.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.rect("UIRect");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
    if (ImGui::MenuItem("UIButton", nullptr, false, can_add_ui)) {
        edit.submit(context.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.button("UIButton");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
    if (ImGui::MenuItem("UIImage", nullptr, false, can_add_ui)) {
        edit.submit(context.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.image("UIImage");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
}

void HierarchyPanel::openAddEntityPopupImpl(const SceneEditContext& context,
                                            const Scene&,
                                            Entity parent) {
    EditService& edit = m_editor_services.edit();

    using CreateFunc = Entity (*)(SceneCommandWriter&, std::string_view);
    auto add_object = [&](const char* name, bool separate, CreateFunc&& create_func) {
        if (ImGui::MenuItem(name)) {
            edit.submit(context.doc_id, [&](SceneCommandWriter& writer) {
                Entity temp = create_func(writer, name);
                writer.attachChild(temp, parent);
                return temp;
            });
        }

        if (separate) {
            ImGui::Separator();
        }
    };

#define DEFINE_OBJECT(NAME, SEP) add_object( \
    #NAME,                                   \
    SEP,                                     \
    [](SceneCommandWriter& writer, std::string_view name) { return writer.NAME##Object(name); });
    OBJECT_LIST
#undef DEFINE_OBJECT

    ImGui::Separator();
    if (ImGui::MenuItem("Canvas", nullptr, false, true)) {
        edit.submit(context.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.canvas("UICanvas");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
}

}  // namespace cave
