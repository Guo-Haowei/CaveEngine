#pragma once
#include "assets/asset_type.h"
#include "assets/guid.h"
#include "ecs/entity.h"

namespace cave {

inline constexpr const char* PAYLOAD_FOLDER = "CAVE/Folder";
inline constexpr const char* PAYLOAD_ASSET = "CAVE/Asset";
inline constexpr const char* PAYLOAD_SCENE_NODE = "CAVE/SceneNode";

class AssetHandle;
struct ContentEntry;

enum class DragKind : uint32_t {
    Asset,
    Folder,
    SceneNode,
};

Option<AssetHandle> DragDropTarget(AssetType p_mask);

template<class T>
inline void SetPayload(const char* p_type, const T& pay_load) {
    ImGui::SetDragDropPayload(p_type, &pay_load, sizeof(T), ImGuiCond_Once);
}

void DragDropSourceContentEntry(const ContentEntry& p_source);

void DragDropTargetFolder(const ContentEntry& p_target,
                          const std::unordered_map<std::string, const ContentEntry*>& p_lut);

}  // namespace cave
