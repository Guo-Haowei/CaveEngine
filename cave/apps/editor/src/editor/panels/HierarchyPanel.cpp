#include "HierarchyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui_internal.h>

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

#include "editor/edit/EditObjectCmd.h"
#include "editor/EditorState.h"
#include "editor/widgets/DragDrop.h"

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

    SceneTreeBuilder(const PreviewScene& preview, SelectionService& selection)
        : m_preview(preview), m_selection(selection) {}

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

    Map<Entity, Owner<HierarchyNode>> m_nodes;
    HierarchyNode* m_root = nullptr;
    const PreviewScene& m_preview;
    SelectionService& m_selection;
};

bool TreeNodeHelper(Scene& scene,
                    Entity ent,
                    ImGuiTreeNodeFlags tree_flags,
                    std::function<void()> on_left_click,
                    std::function<void()> on_right_click,
                    std::function<void()> on_visibility_click) {
    const NameComponent* name_component = scene.component<NameComponent>(ent);
    DEV_ASSERT(name_component);

    std::string_view name = name_component->name();
    if (name.empty()) {
        name = "Untitled";
    }

    const char* icon = ICON_FA_FOLDER;
    if (tree_flags & ImGuiTreeNodeFlags_Leaf) {
        icon = ICON_FA_SQUARE_SHARE_NODES;
    }

    const char* text = ICON_FA_EYE_SLASH;
    //const char* text = ICON_FA_EYE;
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

    // @TODO: refactor to use DragDrop.h interface
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        SetPayload(kPayloadSceneNode, ent);
        ImGui::Text("entity '%.*s'", static_cast<int>(name.size()), name.data());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadSceneNode)) {
            const Entity child_id = *reinterpret_cast<const Entity*>(payload->Data);

            if (child_id != ent) {
                scene.attachChild(child_id, ent);

#if USING(USE_LOG)
                const NameComponent* child_name = scene.component<NameComponent>(child_id);
                if (DEV_VERIFY(child_name)) {
                    LOG_TRACE("moved '{}' under '{}'", child_name->name(), name);
                }
#endif
            }
        }

        ImGui::EndDragDropTarget();
    }

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
        if (on_visibility_click) {
            on_visibility_click();
        }
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

    SelectionKey selection = m_selection.primary(m_preview.doc_id);

    tree_flags |= hier->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
    tree_flags |= current_id == selection.entity ? ImGuiTreeNodeFlags_Selected : 0;

    const bool expanded = TreeNodeHelper(
        *m_preview.scene,
        current_id,
        tree_flags,
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_selection.setSelection(m_preview.doc_id, selection);
        },
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_selection.setSelection(m_preview.doc_id, selection);
            ImGui::OpenPopup(kPopupNameId);
        },
        []() {
            LOG_WARN("TODO: implement");
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
    for (auto [ent, transform] : scene.view<TransformComponent>()) {
        auto find_or_create = [this](Entity ent) -> HierarchyNode* {
            if (ent.isNull()) {
                return nullptr;
            }
            auto [it, ok] = m_nodes.try_emplace(ent, std::make_unique<HierarchyNode>());
            return it->second.get();
        };

        const auto hier = scene.component<HierarchyComponent>(ent);

        const Entity parent_id = hier ? hier->parent_id : Entity::null();
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
        SceneTreeBuilder sceneTree(preview, m_editor_services.selection());
        drawPopup(preview);
        sceneTree.update();
    }
}

void HierarchyPanel::drawPopup(const PreviewScene& preview_scene) {
    if (ImGui::BeginPopup(kPopupNameId)) {
        SelectionKey selection = m_editor_services.selection().primary(preview_scene.doc_id);
        DEV_ASSERT(selection.doc == preview_scene.doc_id);
        ecs::Entity selected = selection.entity;

        ecs::Entity parent = selected.valid() ? selected : preview_scene.scene->root();

        if (ImGui::BeginMenu("Add")) {
            openAddEntityPopupImpl(preview_scene.doc_id, parent);
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

void HierarchyPanel::openAddEntityPopupImpl(DocId doc_id, ecs::Entity parent) {
    DEV_ASSERT(parent.valid());

    using CreateFunc = Entity (*)(SceneCommandWriter&, std::string_view);
    auto add_object = [&](const char* name, bool separate, CreateFunc&& create_func) {
        if (ImGui::MenuItem(name)) {
            m_editor_services.edit().submit(doc_id, [&](SceneCommandWriter& writer) {
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
}

}  // namespace cave
