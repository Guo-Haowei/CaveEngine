#pragma once
#include "DocumentBase.h"

namespace cave {

class MaterialDocument : public DocumentBase {
public:
    MaterialDocument(IApplication& p_app, const Guid& p_guid);

    SceneId GetPreviewScene() const override {
        return m_preview_scene;
    }

protected:
    SceneId m_preview_scene{};
};

}  // namespace cave
