#pragma once
#include "DocumentBase.h"

namespace cave {

class SceneDocument : public DocumentBase {
public:
    SceneDocument(EngineServices& services, const Guid& guid);
    ~SceneDocument();

    bool save() override;

    Owner<Scene> createPreviewScene() const override;

    bool changeProperty(const PropertyTarget& target,
                        const uint8_t* data,
                        size_t data_size) override;
};

}  // namespace cave
