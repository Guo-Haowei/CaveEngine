#pragma once
#include "DocumentBase.h"

namespace cave {

class TileSetDocument : public DocumentBase {
public:
    TileSetDocument(EngineServices& services, const Guid& guid);
};

}  // namespace cave
