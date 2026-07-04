#pragma once
#include "cave/runtime/ecs/ComponentDefines.h"

#include "EditCmdBase.h"

namespace cave {

class ChangePropertyCmd : public EditCmdBase {
    using Self = ChangePropertyCmd;

public:
    ChangePropertyCmd(SceneRegistry& scene_reg,
                      ecs::Entity ent,
                      ComponentId cid,
                      const PropertyId& pid,
                      const void* old_data,
                      const void* new_data,
                      uint32_t data_size);

    template<typename U>
    ChangePropertyCmd(SceneRegistry& scene_reg,
                      ecs::Entity ent,
                      ComponentId cid,
                      const PropertyId& proid,
                      const U& old_value,
                      const U& new_value)
        : ChangePropertyCmd(scene_reg,
                            ent,
                            cid,
                            proid,
                            &old_value,
                            &new_value,
                            sizeof(U)) {
        static_assert(std::is_trivially_copyable_v<U>);
    }

    const char* label() const override {
        return "ChangePropertyCmd";
    }

    bool apply(IDocument& doc) override;

    bool undo(IDocument& doc) override;

    bool canCoalesceWith(const IEditCmd* cmd) const override;

    void coalesceFrom(std::unique_ptr<IEditCmd> cmd) override;

private:
    const ComponentId cid_;
    const PropertyId pid_;

    std::vector<uint8_t> old_;
    std::vector<uint8_t> new_;
};

}  // namespace cave
