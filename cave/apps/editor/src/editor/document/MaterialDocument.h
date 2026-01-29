#pragma once
#include "DocumentBase.h"

namespace cave {

class MaterialDocument : public DocumentBase {
public:
    MaterialDocument(IApplication& p_app, const Guid& p_guid);

    // bool Save() override;
    // bool SaveAs(std::string_view p_new_path) override;

    SceneId GetPreviewScene() const {
        return m_preview_scene;
    }

protected:
    SceneId m_preview_scene{};
};

}  // namespace cave
