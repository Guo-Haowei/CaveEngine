#pragma once
#include "EditCmdBase.h"

#include "engine/private/runtime/scene/Scene.h"
#include "editor/document/SceneDocument.h"

namespace cave {

// @TODO: register default, copy, move constructor and destructor in meta table
template<typename T>
class RemoveComponentCmd : public EditCmdBase {
public:
    RemoveComponentCmd(SceneRegistry& scene_reg, ecs::Entity ent, T& origin)
        : EditCmdBase(scene_reg)
        , m_ent(ent)
        , m_origin(origin) {
    }

    const char* label() const override { return "RemoveComponentCmd"; }

    bool apply(IDocument& doc) override { return Remove(doc); }

    bool undo(IDocument& doc) override { return Add(doc, &m_origin); }

protected:
    bool Add(IDocument& doc, T* value) {
        if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
            if (Scene* scene = resolveScene(scene_doc->previewScene())) {
                if (scene->component<T>(m_ent) == nullptr) {
                    T& comp = scene->create<T>(m_ent);
                    if (value) {
                        comp = *value;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    bool Remove(IDocument& doc) {
        if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&doc)) {
            if (Scene* scene = resolveScene(scene_doc->previewScene())) {
                scene->removeComponent<T>(m_ent);
            }
        }
        return true;
    }

    ecs::Entity m_ent;
    T m_origin;
};

}  // namespace cave
