#pragma once
#include "EditCmdBase.h"

#include "editor/document/SceneDocument.h"

namespace cave {

template<typename T>
class AddComponentCmd : public EditCmdBase {
public:
    using EditCmdBase::EditCmdBase;

    const char* Label() const override { return "AddComponentCmd"; }

    bool Do(IDocument& p_doc) override { return Add(p_doc, nullptr); }

    bool Undo(IDocument& p_doc) override { return Remove(p_doc); }

protected:
    bool Add(IDocument& p_doc, T* p_value) {
        if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
            if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
                if (scene->GetComponent<T>(m_entity) == nullptr) {
                    T& comp = scene->Create<T>(m_entity);
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
                scene->Remove<T>(m_entity);
            }
        }
        return true;
    }
};

template<typename T>
class RemoveComponentCmd : public AddComponentCmd<T> {
public:
    RemoveComponentCmd(IApplication& p_app, ecs::Entity p_ent, T& p_origin)
        : AddComponentCmd<T>(p_app, p_ent)
        , m_origin(p_origin) {
    }

    const char* Label() const override { return "RemoveComponentCmd"; }

    bool Do(IDocument& p_doc) override { return AddComponentCmd<T>::Remove(p_doc); }

    bool Undo(IDocument& p_doc) override { return AddComponentCmd<T>::Add(p_doc, &m_origin); }

protected:
    T m_origin;
};

}  // namespace cave
