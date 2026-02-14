#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/scene/SceneEdit.h"

#include "editor/document/IDocument.h"

namespace cave {

class Scene;

template<typename T>
class EditPropertyCmd : public EditCmdBase {
    using Self = EditPropertyCmd;

public:
    template<typename U>
    EditPropertyCmd(IApplication& p_app,
                    ecs::Entity p_entity,
                    std::string_view p_property,
                    const U& p_old,
                    const U& p_new)
        : EditCmdBase(p_app, p_entity)
        , m_property(p_property) {
        static_assert(std::is_trivially_copyable_v<U>);
        m_old.resize(sizeof(p_old));
        m_new.resize(sizeof(p_new));
        std::memcpy(m_old.data(), &p_old, sizeof(p_old));
        std::memcpy(m_new.data(), &p_new, sizeof(p_new));
    }

    const char* Label() const override {
        return "EditPropertyCmd";
    }

    bool Do(IDocument& p_doc) override {
        SceneId scene_id = p_doc.GetPreviewScene();
        if (!scene_id.IsValid()) return false;
        Scene* scene = ResolveScene(scene_id);
        if (!scene) return false;

        SceneEdit edit(*scene);
        bool res = edit.ModifyField<T>(m_entity,
                                       m_property,
                                       m_new.data(),
                                       (uint32_t)m_new.size());
        return res;
    }

    bool Undo(IDocument& p_doc) override {
        SceneId scene_id = p_doc.GetPreviewScene();
        if (!scene_id.IsValid()) return false;
        Scene* scene = ResolveScene(scene_id);
        if (!scene) return false;

        SceneEdit edit(*scene);
        bool res = edit.ModifyField<T>(m_entity,
                                       m_property,
                                       m_old.data(),
                                       (uint32_t)m_old.size());
        return res;
    }

    bool CanCoalesceWith(const IEditCmd* p_cmd) const override {
        if (const Self* cmd = dynamic_cast<const Self*>(p_cmd)) {
            return cmd->m_entity == cmd->m_entity &&
                   m_property == cmd->m_property;
        }
        return false;
    }

    void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) override {
        Self& cmd = dynamic_cast<Self&>(*p_cmd);
        m_new = std::move(cmd.m_new);
    }

private:
    std::string_view m_property;

    std::vector<uint8_t> m_old;
    std::vector<uint8_t> m_new;
};

}  // namespace cave
