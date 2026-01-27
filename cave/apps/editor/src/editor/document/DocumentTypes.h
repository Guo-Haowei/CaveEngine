#pragma once
#include "cave/runtime/core/GenId.h"

namespace cave {

class IDocument;
using DocId = GenId<IDocument>;

enum class DocKind : uint8_t {
    Scene,
    Script,
    Material,
    Mesh,
    Texture,
    Audio,
    Prefab,
    Shader,

    _Count,
};

}  // namespace cave
