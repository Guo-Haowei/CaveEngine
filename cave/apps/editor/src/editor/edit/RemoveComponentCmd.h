#pragma once
#include "EditCmdBase.h"

#include "engine/private/runtime/scene/Scene.h"
#include "editor/document/SceneDocument.h"

namespace cave {

template<typename T>
class RemoveComponentCmd : public EditCmdBase {
public:
    RemoveComponentCmd(IApplication& p_app, ecs::Entity p_ent, T& p_origin)
        : EditCmdBase(p_app, p_ent)
        , m_origin(p_origin) {
    }

    const char* Label() const override { return "RemoveComponentCmd"; }

    bool Do(IDocument& p_doc) override { return Remove(p_doc); }

    bool Undo(IDocument& p_doc) override { return Add(p_doc, &m_origin); }

protected:
    bool Add(IDocument& p_doc, T* p_value) {
        if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
            if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
                if (scene->GetComponent<T>(m_ent) == nullptr) {
                    T& comp = scene->Create<T>(m_ent);
                    if (p_value) {
                        comp = *p_value;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    bool Remove(IDocument& p_doc) {
        if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
            if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
                scene->Remove<T>(m_ent);
            }
        }
        return true;
    }

    T m_origin;
};

}  // namespace cave
