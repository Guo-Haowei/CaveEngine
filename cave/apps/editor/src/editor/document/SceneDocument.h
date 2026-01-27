#pragma once
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/assets/guid.h"

#include "editor/document/DocumentBase.h"

namespace cave {

class SceneDocument : public DocumentBase {
public:
    SceneDocument(IApplication& p_app, const Guid& p_guid);

    // bool Save() override;

    // SceneId GetSceneId() const { return m_scene_id; }
    bool Save() override;

    bool SaveAs(std::string_view p_new_path) override;
};

}  // namespace cave
