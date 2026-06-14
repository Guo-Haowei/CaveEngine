#pragma once
#include "DocumentBase.h"

namespace cave {

class TileMapAsset;

class TileMapDocument : public DocumentBase {
public:
    TileMapDocument(AppServices& services, const Guid& guid);
};

}  // namespace cave
