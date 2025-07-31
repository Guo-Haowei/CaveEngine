#include "drag_drop.h"

#include "engine/runtime/asset_registry.h"
#include "editor/utility/folder_tree.h"

namespace cave {

namespace fs = std::filesystem;

DragPayload MakePayloadFolder(const std::filesystem::path& p_path) {
    DragPayload payload = {
        .kind = DragKind::Folder,
        .folder = {}
    };
    strncpy(payload.folder.path,
            p_path.string().c_str(),
            sizeof(payload.folder.path) - 1);

    return payload;
}

DragPayload MakePayloadAsset(AssetType p_type, const Guid& p_guid) {
    return {
        .kind = DragKind::Asset,
        .asset = {
            .type = p_type,
            .guid = p_guid,
        },
    };
}

Option<AssetHandle> DragDropTarget(AssetType p_mask) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_ASSET)) {
            const DragPayload& data = *reinterpret_cast<const DragPayload*>(payload->Data);
            DEV_ASSERT(data.kind == DragKind::Asset);
            auto handle = AssetRegistry::GetSingleton().FindByGuid(data.asset.guid, p_mask);
            if (handle.is_some()) {
                return Some(handle.unwrap_unchecked());
            }
        }
        ImGui::EndDragDropTarget();
    }

    return None();
}

static bool IsChild(const ContentEntry* p_node1, const ContentEntry* p_node2) {
    for (const ContentEntry* cursor = p_node1; cursor; cursor = cursor->parent) {
        if (cursor == p_node2) {
            return true;
        }
    }
    return false;
}

void DragDropSourceContentEntry(const ContentEntry& p_source) {
    if (p_source.virtual_path != "@res://") {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            if (p_source.is_dir) {
                DragPayload payload = MakePayloadFolder(p_source.sys_path);
                SetPayload(PAYLOAD_FOLDER, payload);
            } else {
                DragPayload payload = MakePayloadAsset(p_source.type, p_source.handle.GetGuid());
                SetPayload(PAYLOAD_ASSET, payload);
            }
            ImGui::Text("%s", p_source.virtual_path.c_str());
            ImGui::EndDragDropSource();
        }
    }
}

void DragDropTargetFolder(const ContentEntry& p_target,
                          const std::unordered_map<std::string, const ContentEntry*>& p_lut) {
    if (!p_target.is_dir) {
        return;
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_ASSET)) {
            // @TODO: retrieve path
            LOG_WARN("TODO: implement moving asset");
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(PAYLOAD_FOLDER)) {
            const DragPayload& data = *reinterpret_cast<const DragPayload*>(payload->Data);
            auto it = p_lut.find(std::string(data.folder.path));
            DEV_ASSERT(it != p_lut.end());
            const ContentEntry* moved = it->second;
            const bool is_child = IsChild(&p_target, moved);
            if (is_child) {
                LOG_ERROR("can't move '{}' to '{}'", moved->virtual_path, p_target.virtual_path);
            } else {
                fs::path old_path = moved->sys_path;
                fs::path new_path = p_target.sys_path / moved->file_name;
                fs::rename(old_path, new_path);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

}  // namespace cave
