#include "DragDropService.h"

#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/ecs/components/MiscComponents.h"
#include "cave/runtime/framework/EngineServices.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/utility/ContentEntry.h"
#include "editor/services/EditService.h"
#include "editor/services/EditorServices.h"

namespace cave {

namespace fs = std::filesystem;
using ecs::Entity;

namespace {

struct DragPayload {
    DragKind kind;
    Entity entity;
    AssetType type{ AssetType::Unknown };
    Guid guid;
    char path[256]{ 0 };
};

DragPayload MakePayloadFolder(const ContentEntry& entry) {
    DragPayload payload;
    payload.kind = DragKind::Folder,
    strncpy(payload.path,
            entry.sys_path.string().c_str(),
            sizeof(payload.path) - 1);

    return payload;
}

DragPayload MakePayloadAsset(const ContentEntry& entry) {
    DragPayload payload;
    payload.kind = DragKind::Asset;
    payload.type = entry.asset_type;
    payload.guid = entry.handle.guid();
    strncpy(payload.path,
            entry.sys_path.string().c_str(),
            sizeof(payload.path) - 1);

    return payload;
}

bool IsChild(const ContentEntry* node1, const ContentEntry* node2) {
    for (const ContentEntry* cursor = node1; cursor; cursor = cursor->parent) {
        if (cursor == node2) {
            return true;
        }
    }
    return false;
}

}  // namespace

DragDropService::DragDropService(EngineServices& engine_services,
                                 EditorServices& editor_services)
    : m_edit(editor_services.edit())
    , m_scene_reg(engine_services.sceneRegistry()) {
}

void DragDropService::dragSceneNode(Entity ent, std::string_view name) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        SetPayload(kPayloadSceneNode, ent);
        ImGui::Text("entity '%.*s'", static_cast<int>(name.size()), name.data());
        ImGui::EndDragDropSource();
    }
}

void DragDropService::dragContentEntry(const ContentEntry& source) {
    if (source.virtual_path != "@res://") {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            if (source.is_dir) {
                DragPayload payload = MakePayloadFolder(source);
                SetPayload(kPayloadFolder, payload);
            } else {
                DragPayload payload = MakePayloadAsset(source);
                SetPayload(kPayloadAsset, payload);
            }
            ImGui::TextUnformatted(source.virtual_path.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

void DragDropService::dropSceneNode(Entity parent, DocId doc_id, const Scene& scene) {
    auto drop_node = [&]() {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadSceneNode);
        if (!payload) return;

        const Entity child = *reinterpret_cast<const Entity*>(payload->Data);
        if (child.isNull() || child == parent) return;

        auto* child_hier = scene.component<HierarchyComponent>(child);
        if (!DEV_VERIFY(child_hier) || child_hier->parent() == parent) {
            return;
        }

        if (scene.isChild(parent, child)) {
#if USING(USE_LOG)
            const auto* name_parent = scene.component<NameComponent>(parent);
            const auto* name_child = scene.component<NameComponent>(child);
            LOG_ERROR(LogChannel::Scene,
                      "cant' change parent, '{}' is a child of '{}'",
                      name_parent ? name_parent->name() : "??",
                      name_child ? name_child->name() : "??");
#endif
            return;
        }

        // @TODO: need to update tree cache, do not call this
        auto cmd = MakeOwner<ChangePropertyCmd>(
            m_scene_reg,
            child,
            BuiltinComponentId::HierarchyComponent_Id,
            CAVE_SID("parent_id"),
            child_hier->parent(),
            parent);

        m_edit.submit(doc_id, std::move(cmd));
    };

    if (ImGui::BeginDragDropTarget()) {
        drop_node();
        ImGui::EndDragDropTarget();
    }
}

Option<AssetHandle> DragDropService::dropAsset(AssetType mask) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAsset)) {
            const DragPayload& data = *reinterpret_cast<const DragPayload*>(payload->Data);
            DEV_ASSERT(data.kind == DragKind::Asset);
            if (auto handle = AssetRegistry::singleton().findByGuid(data.guid, mask)) {
                return Some(handle.unwrap_unchecked());
            }
        }
        ImGui::EndDragDropTarget();
    }

    return None();
}

void DragDropService::dropFolder(const ContentEntry& target,
                                 const StringHashMap<const ContentEntry*>& lut) {
    if (!target.is_dir) {
        return;
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAsset)) {
            // @TODO: move assets, need to move meta as well
            LOG_WARN("TODO: implement kPayloadAsset");
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadFolder)) {
            const DragPayload& data = *reinterpret_cast<const DragPayload*>(payload->Data);
            auto it = lut.find(data.path);
            DEV_ASSERT(it != lut.end());
            const ContentEntry* moved = it->second;
            const bool is_child = IsChild(&target, moved);
            if (is_child) {
                LOG_ERROR("can't move '{}' to '{}'", moved->virtual_path, target.virtual_path);
            } else {
                fs::path old_path = moved->sys_path;
                fs::path new_path = target.sys_path / moved->file_name;
                fs::rename(old_path, new_path);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

}  // namespace cave
