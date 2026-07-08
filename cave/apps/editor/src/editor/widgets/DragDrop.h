#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/assets/AssetType.h"

namespace cave {

inline constexpr const char* kPayloadFolder = "CAVE/Folder";
inline constexpr const char* kPayloadAsset = "CAVE/Asset";
inline constexpr const char* kPayloadSceneNode = "CAVE/SceneNode";

class AssetHandle;
struct ContentEntry;

enum class DragKind : uint32_t {
    Asset,
    Folder,
    SceneNode,
};

Option<AssetHandle> DragDropTarget(AssetType mask);

template<class T>
inline void SetPayload(const char* type, const T& pay_load) {
    ImGui::SetDragDropPayload(type, &pay_load, sizeof(T), ImGuiCond_Once);
}

void DragDropSourceContentEntry(const ContentEntry& source);

void DragDropTargetFolder(const ContentEntry& target,
                          const std::unordered_map<std::string, const ContentEntry*>& lut);

}  // namespace cave
