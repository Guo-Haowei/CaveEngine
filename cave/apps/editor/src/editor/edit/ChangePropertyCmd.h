#pragma once
#include "EditCmdBase.h"

#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class ChangePropertyCmd : public EditCmdBase {
    using Self = ChangePropertyCmd;

public:
    ChangePropertyCmd(SceneRegistry& p_scene_reg,
                      ecs::Entity p_ent,
                      ComponentId p_cid,
                      const PropertyId& p_pid,
                      const void* p_old_data,
                      const void* p_new_data,
                      uint32_t p_data_size);

    template<typename U>
    ChangePropertyCmd(SceneRegistry& p_scene_reg,
                      ecs::Entity p_ent,
                      ComponentId p_cid,
                      const PropertyId& p_prop_id,
                      const U& p_old,
                      const U& p_new)
        : ChangePropertyCmd(p_scene_reg,
                            p_ent,
                            p_cid,
                            p_prop_id,
                            &p_old,
                            &p_new,
                            sizeof(U)) {
        static_assert(std::is_trivially_copyable_v<U>);
    }

    const char* Label() const override {
        return "ChangePropertyCmd";
    }

    bool Do(IDocument& p_doc) override;

    bool Undo(IDocument& p_doc) override;

    bool CanCoalesceWith(const IEditCmd* p_cmd) const override;

    void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) override;

private:
    const ComponentId m_cid;
    const PropertyId m_pid;

    std::vector<uint8_t> m_old;
    std::vector<uint8_t> m_new;
};

}  // namespace cave
