#pragma once
#include "ChangePropertyCmd.h"

#include "editor/document/IDocument.h"

namespace cave {

ChangePropertyCmd::ChangePropertyCmd(SceneRegistry& scene_reg,
                                     PropertyTarget target,
                                     const void* old_data,
                                     const void* new_data,
                                     uint32_t data_size)
    : EditCmdBase(scene_reg)
    , m_target(target) {
    CRASH_COND_MSG(!new_data, "New property data cannot be null");
    CRASH_COND_MSG(data_size == 0, "Property data cannot be empty");

    m_old.resize(data_size);
    m_new.resize(data_size);

    const void* old = old_data ? old_data : new_data;
    std::memcpy(m_old.data(), old, data_size);
    std::memcpy(m_new.data(), new_data, data_size);
}

bool ChangePropertyCmd::apply(IDocument& doc) {
    return doc.changeProperty(m_target, m_new.data(), m_new.size());
}

bool ChangePropertyCmd::undo(IDocument& doc) {
    return doc.changeProperty(m_target, m_old.data(), m_old.size());
}

bool ChangePropertyCmd::canCoalesceWith(const IEditCmd* edit_cmd) const {
    const Self* cmd = dynamic_cast<const Self*>(edit_cmd);
    return cmd &&
           m_target == cmd->m_target &&
           m_new.size() == cmd->m_new.size();
}

void ChangePropertyCmd::coalesceFrom(Owner<IEditCmd> edit_cmd) {
    Self& cmd = dynamic_cast<Self&>(*edit_cmd);
    m_new = std::move(cmd.m_new);
}

}  // namespace cave
