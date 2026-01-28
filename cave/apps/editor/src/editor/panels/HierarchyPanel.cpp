#include "HierarchyPanel.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui_internal.h>

#include "engine/private/assets/mesh_asset.h"
#include "engine/private/debugger/profiler.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

#include "editor/services/EditService.h"
#include "editor/services/SelectionService.h"

#include "editor/EditorState.h"
#include "editor/viewer/Viewer.h"
#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/DragDrop.h"

namespace cave {
using ecs::Entity;

#define POPUP_NAME_ID "SCENE_PANEL_POPUP"

// @TODO: build the scene tree and attach to scene
// @TODO: on scene change instead of build every frame
class HierarchyCreator {
public:
    struct Ctx {
        SelectionService& selection_service;
        Scene& scene;
        SceneId scene_id;
        DocId doc_id;
    };

    struct HierarchyNode {
        HierarchyNode* parent = nullptr;
        Entity entity;

        std::vector<HierarchyNode*> children;
    };

    HierarchyCreator(const Ctx& p_ctx)
        : m_ctx(p_ctx) {}

    void Update() {
        if (Build(m_ctx.scene)) {
            DEV_ASSERT(m_root);
            DrawNode(m_root, ImGuiTreeNodeFlags_DefaultOpen);
        }
    }

private:
    bool Build(const Scene& p_scene);
    void DrawNode(HierarchyNode* p_node,
                  ImGuiTreeNodeFlags p_flags = 0);

    std::map<Entity, std::shared_ptr<HierarchyNode>> m_nodes;
    HierarchyNode* m_root = nullptr;
    const Ctx& m_ctx;
};

static bool TreeNodeHelper(Scene& p_scene,
                           Entity p_id,
                           ImGuiTreeNodeFlags p_flags,
                           std::function<void()> p_on_left_click,
                           std::function<void()> p_on_right_click) {

    const NameComponent* name_component = p_scene.GetComponent<NameComponent>(p_id);
    std::string name = name_component->GetName();
    if (name.empty()) {
        name = "Untitled";
    }

    const char* icon = ICON_FA_FOLDER;
    if (p_flags & ImGuiTreeNodeFlags_Leaf) {
        icon = ICON_FA_SQUARE_SHARE_NODES;
    }
    auto node_name = std::format("##{}", p_id.GetId());
    auto tag = std::format("{} {}{}", icon, name, node_name);

    p_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const bool expanded = ImGui::TreeNodeEx(node_name.c_str(), p_flags);
    ImGui::SameLine();

    ImGui::Selectable(tag.c_str());
    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (p_on_left_click) {
                p_on_left_click();
            }
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            if (p_on_right_click) {
                p_on_right_click();
            }
        }
    }

    // @TODO: refactor to use DragDrop.h interface
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        SetPayload(PAYLOAD_SCENE_NODE, p_id);
        ImGui::Text("entity '%s'", name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_SCENE_NODE)) {
            Entity child_id = *reinterpret_cast<Entity*>(payload->Data);
            if (child_id != p_id) {
                p_scene.AttachChild(child_id, p_id);

                if constexpr (true) {  // @TODO: log macro
                    const NameComponent* child_name = p_scene.GetComponent<NameComponent>(child_id);
                    DEV_ASSERT(child_name);
                    LOG_VERBOSE("moved '{}' under '{}'", child_name->GetName(), name);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    return expanded;
}

// @TODO: make it an widget
void HierarchyCreator::DrawNode(HierarchyNode* p_hier, ImGuiTreeNodeFlags p_flags) {
    DEV_ASSERT(p_hier);
    Entity current_id = p_hier->entity;

    SelectionKey selection = m_ctx.selection_service.Primary(m_ctx.doc_id);

    p_flags |= p_hier->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
    p_flags |= current_id == selection.entity ? ImGuiTreeNodeFlags_Selected : 0;

    const bool expanded = TreeNodeHelper(
        m_ctx.scene,
        current_id,
        p_flags,
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_ctx.doc_id;
            selection.scene = m_ctx.scene_id;
            selection.entity = current_id;

            m_ctx.selection_service.Set(m_ctx.doc_id, selection);
        },
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_ctx.doc_id;
            selection.scene = m_ctx.scene_id;
            selection.entity = current_id;

            m_ctx.selection_service.Set(m_ctx.doc_id, selection);
            ImGui::OpenPopup(POPUP_NAME_ID);
        });

    if (expanded) {
        float indentWidth = 8.f;
        ImGui::Indent(indentWidth);

        for (auto& child : p_hier->children) {
            DrawNode(child);
        }
        ImGui::Unindent(indentWidth);
    }
}

bool HierarchyCreator::Build(const Scene& p_scene) {
    const size_t hierarchy_count = p_scene.GetCount<HierarchyComponent>();
    if (hierarchy_count == 0) {
        return false;
    }

    for (auto [self_id, hier] : p_scene.View<HierarchyComponent>()) {
        auto find_or_create = [this](ecs::Entity id) {
            auto it = m_nodes.find(id);
            if (it == m_nodes.end()) {
                m_nodes[id] = std::make_shared<HierarchyNode>();
                return m_nodes[id].get();
            }
            return it->second.get();
        };

        const ecs::Entity parent_id = hier.parent_id;
        HierarchyNode* parent_node = find_or_create(parent_id);
        HierarchyNode* self_node = find_or_create(self_id);
        parent_node->children.push_back(self_node);
        parent_node->entity = parent_id;
        self_node->parent = parent_node;
        self_node->entity = self_id;
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
            LOG_ERROR("{} orphan nodes detected", nodes_without_parent - 1);
            s_nodes_without_parent = nodes_without_parent;
        }
    }
    return true;
}

void HierarchyPanel::UpdateInternal(float) {
    CAVE_PROFILE_EVENT();
    if (ViewerTab* tab = m_editor.GetViewer().GetActiveTab(); tab) {
        const SceneId scene_id = tab->GetSceneId();
        const DocId doc_id = tab->GetDocId();
        if (Scene* scene = m_editor.GetApp().GetSceneRegistry()->Resolve(scene_id)) {
            const HierarchyCreator::Ctx ctx = {
                .selection_service = m_editor.SelectionService(),
                .scene = *scene,
                .scene_id = scene_id,
                .doc_id = doc_id,
            };

            HierarchyCreator creator(ctx);
            DrawPopup(tab);
            creator.Update();
        }
    }
}

void HierarchyPanel::DrawPopup(ViewerTab* p_tab) {
    unused(p_tab);
    // auto selected = p_tab->GetSelectedEntity();
    //// @TODO: save commands for undo

    // ViewerTab* tab = m_editor.GetViewer().GetActiveTab();
    // DocId doc = tab ? tab->GetDocId() : DocId{};

    // SceneId scene_id = tab ? tab->GetSceneId() : SceneId{};

    if (ImGui::BeginPopup(POPUP_NAME_ID)) {
        m_editor.OpenAddEntityPopup(ecs::Entity{});

        if (ImGui::MenuItem("Copy")) {
            // if (selected.IsValid()) {
            //     // p_tab->SetCopiedEntity(selected);
            // }
        }
        if (ImGui::MenuItem("Paste")) {
            // if (ecs::Entity to_be_copied = p_tab->GetCopiedEntity(); to_be_copied.IsValid()) {
            //     m_editor.GetEditService().CommandCloneObject(scene_id, to_be_copied);
            // }
        }
        if (ImGui::MenuItem("Delete")) {
            // if (selected.IsValid()) {
            //     p_tab->SetSelectedEntity(Entity::Null());
            //     // move the command to tab document
            //     m_editor.EditService().CommandDeleteObject(scene_id, selected);
            // }
        }
        ImGui::EndPopup();
    }
}

}  // namespace cave
