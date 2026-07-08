#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

#include "editor/document/DocId.h"

namespace cave {

class EditorState;

enum class SelectionKind : uint8_t {
    None = 0,
    Scene,
    Entity,
    Asset,
    Tile,
};

struct SelectionKey {
    SelectionKind kind{ SelectionKind::None };
    DocId doc{};
    SceneId scene{};
    ecs::Entity entity{};

    // Optional: sub-selection inside an object (future).
    // E.g. component type, tile coord, face index.
    uint32_t sub_kind{};
    uint64_t sub_id{};

    bool IsValid() const { return kind != SelectionKind::None; }

    friend bool operator==(const SelectionKey&, const SelectionKey&) = default;
};

class SelectionService {
public:
    SelectionService(EditorState& p_editor)
        : m_editor(p_editor) {}

    void Set(DocId p_doc_id, const SelectionKey& p_key);

    SelectionKey Primary(DocId p_doc_id);

private:
    EditorState& m_editor;

    std::unordered_map<DocId, SelectionKey> m_selections;
};

}  // namespace cave
