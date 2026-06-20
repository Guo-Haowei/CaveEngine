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
using ecs::Entity;

#define POPUP_NAME_ID "SCENE_PANEL_POPUP"

// @TODO: build the scene tree and attach to scene
// @TODO: on scene change instead of build every frame
class HierarchyCreator {
public:
    struct HierarchyNode {
        HierarchyNode* parent = nullptr;
        Entity entity;

        std::vector<HierarchyNode*> children;
    };

    HierarchyCreator(const PreviewScene& p_preview, SelectionService& p_selection)
        : m_preview(p_preview), m_selection(p_selection) {}

    void Update() {
        if (Build(*m_preview.scene)) {
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
    const PreviewScene& m_preview;
    SelectionService& m_selection;
};

static bool TreeNodeHelper(Scene& p_scene,
                           Entity p_id,
                           ImGuiTreeNodeFlags p_flags,
                           std::function<void()> p_on_left_click,
                           std::function<void()> p_on_right_click) {

    const NameComponent* name_component = p_scene.component<NameComponent>(p_id);
    std::string_view name = name_component->GetName();
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
        ImGui::Text("entity '%s'", name.data());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_SCENE_NODE)) {
            Entity child_id = *reinterpret_cast<Entity*>(payload->Data);
            if (child_id != p_id) {
                p_scene.attachChild(child_id, p_id);

                if constexpr (true) {  // @TODO: log macro
                    const NameComponent* child_name = p_scene.component<NameComponent>(child_id);
                    DEV_ASSERT(child_name);
                    LOG_TRACE("moved '{}' under '{}'", child_name->GetName(), name);
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

    SelectionKey selection = m_selection.Primary(m_preview.doc_id);

    p_flags |= p_hier->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
    p_flags |= current_id == selection.entity ? ImGuiTreeNodeFlags_Selected : 0;

    const bool expanded = TreeNodeHelper(
        *m_preview.scene,
        current_id,
        p_flags,
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_selection.Set(m_preview.doc_id, selection);
        },
        [this, current_id]() {
            SelectionKey selection;
            selection.kind = SelectionKind::Entity;
            selection.doc = m_preview.doc_id;
            selection.scene = m_preview.scene_id;
            selection.entity = current_id;

            m_selection.Set(m_preview.doc_id, selection);
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
    const size_t hierarchy_count = p_scene.count<HierarchyComponent>();
    if (hierarchy_count == 0) {
        return false;
    }

    for (auto [self_id, hier] : p_scene.view<HierarchyComponent>()) {
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

void HierarchyPanel::drawUIImpl() {
    CAVE_PROFILE_EVENT();
    PreviewScene preview = editor_services_.workspace().focusedPreviewScene();
    if (preview.scene) {
        HierarchyCreator creator(preview, editor_services_.selection());
        drawPopup(preview);
        creator.Update();
    }
}

void HierarchyPanel::drawPopup(const PreviewScene& p_ctx) {
    if (ImGui::BeginPopup(POPUP_NAME_ID)) {
        SelectionKey selection = editor_services_.selection().Primary(p_ctx.doc_id);
        DEV_ASSERT(selection.doc == p_ctx.doc_id);
        ecs::Entity selected = selection.entity;

        ecs::Entity parent = selected.IsValid() ? selected : p_ctx.scene->m_root;

        if (ImGui::BeginMenu("Add")) {
            openAddEntityPopupImpl(p_ctx.doc_id, parent);
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
            if (selected.IsValid()) {
                auto cmd = std::make_unique<DeleteObjectCmd>(
                    engine_services_.sceneRegistry(),
                    selected);
                editor_services_.edit().submit(p_ctx.doc_id, std::move(cmd));
            }
        }
        ImGui::EndPopup();
    }
}

// clang-format off
#define OBJECT_LIST                      \
    DEFINE_OBJECT(InfiniteLight,  false) \
    DEFINE_OBJECT(PointLight,     false) \
    DEFINE_OBJECT(AreaLight,      true ) \
    DEFINE_OBJECT(Transform,      false) \
    DEFINE_OBJECT(Plane,          false) \
    DEFINE_OBJECT(Cube,           false) \
    DEFINE_OBJECT(Sphere,         false) \
    DEFINE_OBJECT(Cylinder,       false) \
    DEFINE_OBJECT(Cone,           false) \
    DEFINE_OBJECT(Torus,          true )
// clang-format on

void HierarchyPanel::openAddEntityPopupImpl(DocId p_doc_id, ecs::Entity p_parent) {
    DEV_ASSERT(p_parent.IsValid());

    using CreateFunc = Entity (*)(SceneCommandWriter& p_cb, std::string_view p_name);
    auto add_object = [&](const char* p_name, bool p_separator, CreateFunc p_func) {
        if (ImGui::MenuItem(p_name)) {
            editor_services_.edit().submit(p_doc_id, [&](SceneCommandWriter& cb) {
                Entity temp = p_func(cb, p_name);
                cb.AttachChild(temp, p_parent);
            });
        }

        if (p_separator) {
            ImGui::Separator();
        }
    };

#define DEFINE_OBJECT(NAME, SEP) add_object( \
    #NAME,                                   \
    SEP,                                     \
    [](SceneCommandWriter& p_cb, std::string_view p_name) { return p_cb.Create##NAME##Object(p_name); });
    OBJECT_LIST
#undef DEFINE_OBJECT
}

}  // namespace cave
