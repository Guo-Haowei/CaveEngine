#pragma once
#include "DocumentBase.h"

namespace cave {

class SceneDocument : public DocumentBase {
public:
    SceneDocument(EngineServices& services, const Guid& guid);
    ~SceneDocument();

    bool save() override;
    bool saveAs(std::string_view new_path) override;

    Owner<Scene> createPreviewScene() const override;
};

}  // namespace cave
