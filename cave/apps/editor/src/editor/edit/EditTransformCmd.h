#pragma once
#include "EditCmdBase.h"

// @TODO: move to public
#include "engine/private/math/geomath.h"

namespace cave {

class EditTransformCmd : public IEditCmd {
public:
    EditTransformCmd(const EditCmdCtx& p_ctx,
                     const Matrix4x4f& p_before,
                     const Matrix4x4f& p_after)
        : m_ctx(p_ctx)
        , m_before(p_before)
        , m_after(p_after) {}

    const char* Label() const override {
        return "IEditTransformCmd";
    }

    bool Do(IDocument& p_doc) override;
    bool Undo(IDocument& p_doc) override;

    bool CanCoalesceWith(const IEditCmd* p_cmd) const override;
    void CoalesceFrom(std::unique_ptr<IEditCmd> p_cmd) override;

private:
    EditCmdCtx m_ctx;
    Matrix4x4f m_before;
    Matrix4x4f m_after;
};

}  // namespace cave
