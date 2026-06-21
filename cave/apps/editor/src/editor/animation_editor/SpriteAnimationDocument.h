#pragma once
#include "editor/document/DocumentBase.h"

namespace cave {

class SpriteAnimationDocument : public DocumentBase {
public:
    SpriteAnimationDocument(EngineServices& services, const Guid& guid);
};

}  // namespace cave
