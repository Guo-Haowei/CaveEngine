#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/core/reflection/MetaRegistry.h"

#include "editor/edit/EditCmdBase.h"
#include "editor/document/IDocument.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/framework/Engine.h"

namespace cave {

template<typename ValueT>
class ChangeObjectPropertyCmd : public EditCmdBase {
public:
    ChangeObjectPropertyCmd(SceneRegistry& scene_reg,
                            ecs::Entity ent,
                            ComponentId cid,
                            PropertyId pid,
                            ValueT&& old_value,
                            ValueT&& new_value)
        : EditCmdBase(scene_reg, ent)
        , m_cid(cid)
        , m_pid(pid)
        , m_old(std::move(old_value))
        , m_new(std::move(new_value)) {
    }

    const char* label() const override {
        return "ChangeObjectPropertyCmd";
    }

    bool apply(IDocument& doc) override {
        return setValue(doc, m_new);
    }

    bool undo(IDocument& doc) override {
        return setValue(doc, m_old);
    }

private:
    bool setValue(IDocument& doc, const ValueT& value) {
        SceneId scene_id = doc.previewScene();
        if (!scene_id.valid()) return false;
        Scene* scene = resolveScene(scene_id);
        if (!scene) return false;

        void* component = scene->storage().getRaw(m_cid, m_ent);
        if (!component) return false;

        const auto& reg = engine::GetComponentRegistry();
        const auto* meta = reg.tryGet(m_cid);
        if (!meta) return false;

        const auto* prop = meta->find(m_pid);
        if (!prop) return false;

        prop->template GetData<ValueT>(component) = value;
        return true;
    }

private:
    ComponentId m_cid;
    PropertyId m_pid;

    ValueT m_old;
    ValueT m_new;
};

}  // namespace cave