#pragma once
#include "DocumentBase.h"

namespace cave {

class TileMapAsset;

class TileMapDocument : public DocumentBase {
public:
    TileMapDocument(EngineServices& services, const Guid& guid);

    std::unique_ptr<Scene> createPreviewScene() const override;
};

}  // namespace cave
