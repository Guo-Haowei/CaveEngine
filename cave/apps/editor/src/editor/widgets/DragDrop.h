#pragma once
#include "cave/core/containers/StringHash.h"
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

template<class T>
inline void SetPayload(const char* type, const T& pay_load) {
    ImGui::SetDragDropPayload(type, &pay_load, sizeof(T), ImGuiCond_Once);
}

void DragDropSource_SceneNode(ecs::Entity ent, std::string_view name);

void DragDropSource_ContentEntry(const ContentEntry& source);

void DragDropTarget_SceneNode(ecs::Entity ent, Scene& scene);

void DragDropTarget_Folder(const ContentEntry& target,
                           const StringHashMap<const ContentEntry*>& lut);

Option<AssetHandle> DragDropTarget_Asset(AssetType mask);

}  // namespace cave
