#pragma once
#include "assets/asset_type.h"
#include "assets/guid.h"
#include "ecs/entity.h"

namespace cave {

inline constexpr const char* PAYLOAD_FOLDER = "CAVE/Folder";
inline constexpr const char* PAYLOAD_ASSET = "CAVE/Asset";
inline constexpr const char* PAYLOAD_SCENE_NODE = "CAVE/SceneNode";

class AssetHandle;

enum class DragKind : uint32_t {
    Asset,
    Folder,
    SceneNode,
};

struct DragPayloadFolder {
    char path[256];
};

struct DragPayloadAsset {
    AssetType type;
    Guid guid;
};

struct DragPayloadEntity {
    ecs::Entity entity;
    Guid scene_id;
};

struct DragPayload {
    DragKind kind;
    union {
        DragPayloadFolder folder;
        DragPayloadAsset asset;
        DragPayloadEntity node;
    };
};

DragPayload MakePayloadFolder(const std::filesystem::path& p_path);

DragPayload MakePayloadAsset(AssetType p_type, const Guid& p_guid);

template<class T>
inline void SetPayload(const char* p_type, const T& pay_load) {
    ImGui::SetDragDropPayload(p_type, &pay_load, sizeof(T), ImGuiCond_Once);
}

Option<AssetHandle> DragDropTarget(AssetType p_mask);

}  // namespace cave
