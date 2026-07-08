#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"

#include "EditCmdBase.h"

#include "engine/private/runtime/scene/Scene.h"
#include "editor/document/IDocument.h"
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
        , cid_(cid)
        , pid_(pid)
        , old_(std::move(old_value))
        , new_(std::move(new_value)) {
    }

    const char* label() const override {
        return "ChangeObjectPropertyCmd";
    }

    bool apply(IDocument& doc) override {
        return setValue(doc, new_);
    }

    bool undo(IDocument& doc) override {
        return setValue(doc, old_);
    }

private:
    bool setValue(IDocument& doc, const ValueT& value) {
        SceneId scene_id = doc.previewScene();
        if (!scene_id.valid()) return false;
        Scene* scene = resolveScene(scene_id);
        if (!scene) return false;

        void* component = scene->storage().getRaw(cid_, m_ent);
        if (!component) return false;

        const auto& reg = engine::GetComponentRegistry();
        const auto* meta = reg.tryGet(cid_);
        if (!meta) return false;

        const auto* prop = meta->find(pid_);
        if (!prop) return false;

        prop->template GetData<ValueT>(component) = value;
        return true;
    }

private:
    ComponentId cid_;
    PropertyId pid_;

    ValueT old_;
    ValueT new_;
};

}  // namespace cave