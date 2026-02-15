#pragma once
#include "EditCmdBase.h"

namespace cave {

class Scene;

class EditPropertyCmd : public EditCmdBase {
    using Self = EditPropertyCmd;

public:
    template<typename U>
    EditPropertyCmd(IApplication& p_app,
                    ecs::Entity p_ent,
                    ComponentId p_id,
                    const PropertyId& p_prop_id,
                    const U& p_old,
                    const U& p_new)
        : EditCmdBase(p_app, p_ent)
        , m_id(p_id)
        , m_prop_id(p_prop_id) {
        static_assert(std::is_trivially_copyable_v<U>);
        m_old.resize(sizeof(p_old));
        m_new.resize(sizeof(p_new));
        std::memcpy(m_old.data(), &p_old, sizeof(p_old));
        std::memcpy(m_new.data(), &p_new, sizeof(p_new));
    }

    const char* Label() const override {
        return "EditPropertyCmd";
    }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument& p_doc) override;

    bool CanCoalesceWith(const IEditCmd* p_cmd) const override;

    void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) override;

private:
    const ComponentId m_id;
    const PropertyId m_prop_id;

    std::vector<uint8_t> m_old;
    std::vector<uint8_t> m_new;
};

}  // namespace cave
