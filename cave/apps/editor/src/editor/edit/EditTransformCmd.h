#pragma once
#include "EditCmdBase.h"

#include "cave/core/math/Matrix.h"

namespace cave {

class EditTransformCmd : public EditCmdBase {
public:
    EditTransformCmd(IApplication& p_app,
                     ecs::Entity p_ent,
                     const math::Matrix4x4f& p_before,
                     const math::Matrix4x4f& p_after)
        : EditCmdBase(p_app, p_ent)
        , m_before(p_before)
        , m_after(p_after) {}

    const char* Label() const override {
        return "EditTransformCmd";
    }

    bool Do(IDocument& p_doc) override;
    bool Undo(IDocument& p_doc) override;

    bool CanCoalesceWith(const IEditCmd* p_cmd) const override;
    void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) override;

private:
    math::Matrix4x4f m_before;
    math::Matrix4x4f m_after;
};

}  // namespace cave
