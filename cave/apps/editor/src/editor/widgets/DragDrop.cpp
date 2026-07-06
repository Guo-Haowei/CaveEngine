#include "DragDrop.h"

#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/utility/ContentEntry.h"

namespace cave {

namespace fs = std::filesystem;

namespace {

struct DragPayload {
    DragKind kind;
    ecs::Entity entity;
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
    payload.type = entry.type;
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

Option<AssetHandle> DragDropTarget(AssetType mask) {
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

void DragDropSourceContentEntry(const ContentEntry& source) {
    if (source.virtual_path != "@res://") {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            if (source.is_dir) {
                DragPayload payload = MakePayloadFolder(source);
                SetPayload(kPayloadFolder, payload);
            } else {
                DragPayload payload = MakePayloadAsset(source);
                SetPayload(kPayloadAsset, payload);
            }
            ImGui::Text("%s", source.virtual_path.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

void DragDropTargetFolder(const ContentEntry& target,
                          const std::unordered_map<std::string, const ContentEntry*>& lut) {

    if (!target.is_dir) {
        return;
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadAsset)) {
            // @TODO: move assets, need to move meta as well
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kPayloadFolder)) {
            const DragPayload& data = *reinterpret_cast<const DragPayload*>(payload->Data);
            auto it = lut.find(std::string(data.path));
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
