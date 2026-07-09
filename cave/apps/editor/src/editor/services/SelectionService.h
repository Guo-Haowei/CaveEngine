#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

#include "editor/document/DocId.h"

namespace cave {

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

    bool valid() const { return kind != SelectionKind::None; }

    friend bool operator==(const SelectionKey&, const SelectionKey&) = default;
};

class SelectionService {
public:
    void setSelection(DocId doc_id, const SelectionKey& key);

    SelectionKey primary(DocId doc_id);

private:
    std::unordered_map<DocId, SelectionKey> m_selections;
};

}  // namespace cave
