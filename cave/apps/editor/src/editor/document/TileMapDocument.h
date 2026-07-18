#pragma once
#include "DocumentBase.h"

namespace cave {

class TileMapAsset;

class TileMapDocument : public DocumentBase {
public:
    TileMapDocument(EngineServices& services, const Guid& guid);
};

}  // namespace cave
