#pragma once
#include <variant>

#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/ecs/ComponentDefines.h"

#include "EditCmdBase.h"

namespace cave {

struct PropertyPathStep {
    PropertyId property_id;
    Option<uint32_t> index;

    bool operator==(const PropertyPathStep&) const = default;
};

using PropertyPath = Vector<PropertyPathStep>;

struct ComponentPropertyTarget {
    ecs::Entity entity;
    ComponentId cid;
    PropertyId pid;

    bool operator==(const ComponentPropertyTarget&) const = default;
};

struct AssetPropertyTarget {
    // Relative to the root asset owned by this document.
    PropertyPath path;

    bool operator==(const AssetPropertyTarget&) const = default;
};

using PropertyTarget = std::variant<ComponentPropertyTarget, AssetPropertyTarget>;

class ChangePropertyCmd : public EditCmdBase {
    using Self = ChangePropertyCmd;

public:
    ChangePropertyCmd(SceneRegistry& scene_reg,
                      PropertyTarget target,
                      const void* old_data,
                      const void* new_data,
                      uint32_t data_size);

    template<typename U>
    ChangePropertyCmd(SceneRegistry& scene_reg,
                      PropertyTarget target,
                      const U& old_value,
                      const U& new_value)
        : ChangePropertyCmd(scene_reg,
                            target,
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

    void coalesceFrom(Owner<IEditCmd> cmd) override;

private:
    PropertyTarget m_target;

    Vector<uint8_t> m_old;
    Vector<uint8_t> m_new;
};

}  // namespace cave
