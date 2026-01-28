#pragma once
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/ecs/entity.h"

#include "editor/document/DocumentTypes.h"

namespace cave {

class EditorState;

enum class SelectionKind : uint8_t {
    None,
    Scene,
    Entity,
    Asset,
    // later: Component, MeshFace, Tile, Bone, etc.
};

struct SelectionKey {
    SelectionKind kind = SelectionKind::None;

    DocId doc{};
    SceneId scene{};
    ecs::Entity entity{};

    // Optional: sub-selection inside an object (future).
    // E.g. component type, tile coord, face index.
    uint32_t sub_kind{};
    uint64_t sub_id{};

    friend bool operator==(const SelectionKey&, const SelectionKey&) = default;
};

class SelectionService {
public:
    SelectionService(EditorState& p_editor)
        : m_editor(p_editor) {}

private:
    EditorState& m_editor;
};

}  // namespace cave
