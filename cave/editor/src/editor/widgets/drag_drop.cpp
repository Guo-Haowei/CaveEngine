#include "drag_drop.h"

#include "engine/runtime/asset_registry.h"

namespace cave {

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

}  // namespace cave
