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

#define POPUP_NAME_ID "SCENE_PANEL_POPUP"

namespace {

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

bool TreeNodeHelper(Scene& scene,
                    Entity ent,
                    ImGuiTreeNodeFlags tree_flags,
                    std::function<void()> on_left_click,
                    std::function<void()> on_right_click) {
    const NameComponent* name_component = scene.component<NameComponent>(ent);
    std::string_view name = name_component->name();
    if (name.empty()) {
        name = "Untitled";
    }

    const char* icon = ICON_FA_FOLDER;
    if (tree_flags & ImGuiTreeNodeFlags_Leaf) {
        icon = ICON_FA_SQUARE_SHARE_NODES;
    }
    auto node_name = std::format("##{}", ent.id());
    auto tag = std::format("{} {}{}", icon, name, node_name);

    tree_flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const bool expanded = ImGui::TreeNodeEx(node_name.c_str(), tree_flags);
    ImGui::SameLine();

    ImGui::Selectable(tag.c_str());
    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (on_left_click) {
                on_left_click();
            }
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            if (on_right_click) {
                on_right_click();
            }
        }
    }

    // @TODO: refactor to use DragDrop.h interface
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        SetPayload(kPayloadSceneNode, ent);
        ImGui::Text("entity '%s'", name.data());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadSceneNode)) {
            Entity child_id = *reinterpret_cast<Entity*>(payload->Data);
            if (child_id != ent) {
                scene.attachChild(child_id, ent);

                if constexpr (true) {  // @TODO: log macro
                    const NameComponent* child_name = scene.component<NameComponent>(child_id);
                    DEV_ASSERT(child_name);
                    LOG_TRACE("moved '{}' under '{}'", child_name->name(), name);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    return expanded;
}

}  // namespace

// @TODO: make it an widget
void HierarchyCreator::DrawNode(HierarchyNode* hier, ImGuiTreeNodeFlags tree_flags) {
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
            ImGui::OpenPopup(POPUP_NAME_ID);
        });

    if (expanded) {
        float indentWidth = 8.f;
        ImGui::Indent(indentWidth);

        for (auto& child : hier->children) {
            DrawNode(child);
        }
        ImGui::Unindent(indentWidth);
    }
}

bool HierarchyCreator::Build(const Scene& scene) {
    const size_t hierarchy_count = scene.count<HierarchyComponent>();
    if (hierarchy_count == 0) {
        return false;
    }

    for (auto [self_id, hier] : scene.view<HierarchyComponent>()) {
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
    PreviewScene preview = m_editor_services.workspace().focusedPreviewScene();
    if (preview.scene) {
        HierarchyCreator creator(preview, m_editor_services.selection());
        drawPopup(preview);
        creator.Update();
    }
}

void HierarchyPanel::drawPopup(const PreviewScene& preview_scene) {
    if (ImGui::BeginPopup(POPUP_NAME_ID)) {
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
                auto cmd = std::make_unique<DeleteObjectCmd>(
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
