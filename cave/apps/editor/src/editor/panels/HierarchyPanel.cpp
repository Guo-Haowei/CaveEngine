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
    struct HierarchyNode {
        HierarchyNode* parent = nullptr;
        Entity entity;

        Vector<HierarchyNode*> children;
    };

    SceneTreeBuilder(const PreviewScene& preview,
                     EngineServices& engine_services,
                     EditorServices& editor_services)
        : m_preview(preview)
        , m_engine_services(engine_services)
        , m_editor_services(editor_services) {}

    void update() {
        if (buildSceneTree(*m_preview.scene)) {
            DEV_ASSERT(m_root);
            drawNode(m_root, ImGuiTreeNodeFlags_DefaultOpen);
        }
    }

private:
    bool buildSceneTree(const Scene& scene);
    void drawNode(HierarchyNode* node,
                  ImGuiTreeNodeFlags flags = 0);
    bool treeNodeHelper(Scene& scene,
                        Entity ent,
                        ImGuiTreeNodeFlags tree_flags,
                        std::function<void()> on_left_click,
                        std::function<void()> on_right_click);

    const PreviewScene& m_preview;

    EngineServices& m_engine_services;
    EditorServices& m_editor_services;

    Map<Entity, Owner<HierarchyNode>> m_nodes;
    HierarchyNode* m_root = nullptr;
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
    m_editor_services.dragDrop().dropSceneNode(ent, m_preview.doc_id, scene);

    const float visibility_x =
        ImGui::GetWindowContentRegionMax().x -
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
            ent,
            BuiltinComponentId::HierarchyComponent_Id,
            CAVE_SID("local_visible"),
            hier_component->localVisible(),
            !hier_component->localVisible());

        m_editor_services.edit().submit(m_preview.doc_id, std::move(cmd));
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
void SceneTreeBuilder::drawNode(HierarchyNode* hier, ImGuiTreeNodeFlags tree_flags) {
    DEV_ASSERT(hier);
    Entity current_id = hier->entity;

    SelectionKey selection = m_editor_services.selection().primary(m_preview.doc_id);

    tree_flags |= hier->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
    tree_flags |= current_id == selection.entity ? ImGuiTreeNodeFlags_Selected : 0;

    const bool expanded = treeNodeHelper(
        *m_preview.scene,
        current_id,
        tree_flags,
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_editor_services.selection().setSelection(m_preview.doc_id, selection);
        },
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_editor_services.selection().setSelection(m_preview.doc_id, selection);
            ImGui::OpenPopup(kPopupNameId);
        });

    if (expanded) {
        float indentWidth = 8.f;
        ImGui::Indent(indentWidth);

        for (auto& child : hier->children) {
            drawNode(child);
        }
        ImGui::Unindent(indentWidth);
    }
}

bool SceneTreeBuilder::buildSceneTree(const Scene& scene) {
    for (auto [ent, hier] : scene.view<HierarchyComponent>()) {
        auto find_or_create = [this](Entity ent) -> HierarchyNode* {
            if (ent.isNull()) {
                return nullptr;
            }
            auto [it, ok] = m_nodes.try_emplace(ent, MakeOwner<HierarchyNode>());
            return it->second.get();
        };

        const Entity parent_id = hier.parent();
        HierarchyNode* parent_node = find_or_create(parent_id);
        HierarchyNode* self_node = find_or_create(ent);
        if (parent_node) {
            parent_node->children.push_back(self_node);
            parent_node->entity = parent_id;
        }
        if (DEV_VERIFY(self_node)) {
            self_node->parent = parent_node;
            self_node->entity = ent;
        }
    }

    int nodes_without_parent = 0;
    for (auto& it : m_nodes) {
        if (!it.second->parent) {
            ++nodes_without_parent;
            m_root = it.second.get();
        }
    }

    if (nodes_without_parent != 1) {
        static int s_nodes_without_parent = 0;
        if (nodes_without_parent != s_nodes_without_parent) {
            LOG_ERROR(LogChannel::Scene, "{} orphan nodes detected", nodes_without_parent - 1);
            s_nodes_without_parent = nodes_without_parent;
        }
    }
    return true;
}

void HierarchyPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();
    PreviewScene preview = m_editor_services.workspace().focusedPreviewScene();
    if (preview.scene) {
        SceneTreeBuilder sceneTree(preview,
                                   m_engine_services,
                                   m_editor_services);
        drawPopup(preview);
        sceneTree.update();
    }
}

void HierarchyPanel::drawPopup(const PreviewScene& preview_scene) {
    // @TODO: refactor this

    if (ImGui::BeginPopup(kPopupNameId)) {
        SelectionKey selection = m_editor_services.selection().primary(preview_scene.doc_id);
        DEV_ASSERT(selection.doc == preview_scene.doc_id);
        ecs::Entity selected = selection.entity;

        ecs::Entity parent = selected.valid() ? selected : preview_scene.scene->root();

        if (ImGui::BeginMenu("Add")) {
            const bool is_ui = preview_scene.scene->has(UICanvasComponent_Id, parent) ||
                               preview_scene.scene->has(UIRectTransformComponent_Id, parent);
            if (is_ui) {
                openAddUIPopupImpl(preview_scene, parent);
            } else {
                openAddEntityPopupImpl(preview_scene, parent);
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
                m_editor_services.edit().submit(preview_scene.doc_id, std::move(cmd));
            }
        }
        if (ImGui::MenuItem("Save as Prefab")) {
            PrefabExporter exporter;
            exporter.exportPrefab("@res://exported.prefab",
                                  *preview_scene.scene,
                                  selected);
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

void HierarchyPanel::openAddUIPopupImpl(const PreviewScene& preview_scene, ecs::Entity parent) {
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

    const bool can_add_ui = can_add_canvas_item(*preview_scene.scene, parent);

    if (ImGui::MenuItem("UIRect", nullptr, false, can_add_ui)) {
        edit.submit(preview_scene.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.rect("UIRect");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
    if (ImGui::MenuItem("UIButton", nullptr, false, can_add_ui)) {
        edit.submit(preview_scene.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.button("UIButton");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
    if (ImGui::MenuItem("UIImage", nullptr, false, can_add_ui)) {
        edit.submit(preview_scene.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.image("UIImage");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
}

void HierarchyPanel::openAddEntityPopupImpl(const PreviewScene& preview_scene, ecs::Entity parent) {
    EditService& edit = m_editor_services.edit();

    using CreateFunc = Entity (*)(SceneCommandWriter&, std::string_view);
    auto add_object = [&](const char* name, bool separate, CreateFunc&& create_func) {
        if (ImGui::MenuItem(name)) {
            edit.submit(preview_scene.doc_id, [&](SceneCommandWriter& writer) {
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
    const bool is_parent_root = parent == preview_scene.scene->root();
    if (ImGui::MenuItem("Canvas", nullptr, false, is_parent_root)) {
        edit.submit(preview_scene.doc_id, [&](SceneCommandWriter& writer) {
            Entity temp = writer.canvas("UICanvas");
            writer.attachChild(temp, parent);
            return temp;
        });
    }
}

}  // namespace cave
