#pragma once
#include "cave/core/containers/StringHash.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/assets/AssetType.h"

#include "editor/document/DocId.h"

namespace cave {

inline constexpr const char* kPayloadFolder = "CAVE/Folder";
inline constexpr const char* kPayloadAsset = "CAVE/Asset";
inline constexpr const char* kPayloadSceneNode = "CAVE/SceneNode";

class AssetHandle;
class EditService;
class Scene;
class SceneRegistry;

struct EditorServices;
struct EngineServices;
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

// void DragDropTarget_SceneNode(ecs::Entity ent, Scene& scene);

class DragDropService {
public:
    DragDropService(EngineServices& engine_services,
                    EditorServices& editor_services);

    void dragSceneNode(ecs::Entity ent, std::string_view name);

    void dragContentEntry(const ContentEntry& source);

    void dropSceneNode(ecs::Entity ent, DocId doc_id, const Scene& scene);

    void dropFolder(const ContentEntry& target,
                    const StringHashMap<const ContentEntry*>& lut);

    Option<AssetHandle> dropAsset(AssetType mask);

private:
    EditService& m_edit;
    SceneRegistry& m_scene_reg;
};

}  // namespace cave
