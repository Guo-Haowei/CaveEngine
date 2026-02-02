#pragma once
#include "DocumentBase.h"

namespace cave {

class SceneDocument : public DocumentBase {
public:
    SceneDocument(IApplication& p_app, const Guid& p_guid);

    bool Save() override;
    bool SaveAs(std::string_view p_new_path) override;

    SceneId GetPreviewScene() const override {
        return m_preview_scene;
    }

protected:
    SceneId m_preview_scene{};
};

}  // namespace cave
