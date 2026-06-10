#include "DocumentBase.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/AssetRegistry.h"

#define DEBUG_DOC IN_USE
#if USING(DEBUG_DOC)
#define DEBUG_DOC_LOG(...) LOG_TRACE(__VA_ARGS__)
#else
#define DEBUG_DOC_LOG(...) (void)0
#endif

namespace cave {

DocumentBase::DocumentBase(IApplication& app, const Guid& guid)
    : asset_reg_(*app.GetAssetRegistry())
    , scene_reg_(app.services().sceneRegistry())
    , guid_(guid) {

    handle_ = asset_reg_.FindByGuid(guid).unwrap();
    asset_ = handle_.Wait();
}

bool DocumentBase::apply(std::unique_ptr<IEditCmd> cmd, uint32_t coalesce) {
    if (!cmd) return false;

    if (!undo_.empty() /*&& coalesce != 0 && last_coalesce_ == coalesce*/) {
        IEditCmd* last = undo_.back().get();
        if (last && last->CanCoalesceWith(cmd.get())) {
            cmd->Do(*this);
            last->CoalesceFrom(std::move(cmd));
            redo_.clear();
            touchDirtyAfterEdit();
            return true;
        }
    }

    cmd->Do(*this);

    undo_.push_back(std::move(cmd));
    redo_.clear();

    last_coalesce_ = coalesce;
    touchDirtyAfterEdit();
    trimUndoIfNeeded();

    return true;
}

bool DocumentBase::undo() {
    if (undo_.empty()) return false;
    auto cmd = std::move(undo_.back());
    undo_.pop_back();

    cmd->Undo(*this);
    redo_.push_back(std::move(cmd));

    last_coalesce_ = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

bool DocumentBase::redo() {
    if (redo_.empty()) return false;
    auto cmd = std::move(redo_.back());
    redo_.pop_back();

    cmd->Do(*this);
    undo_.push_back(std::move(cmd));

    last_coalesce_ = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

void DocumentBase::undoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = undo_.rbegin(); it != undo_.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->Label());
    }
}

void DocumentBase::redoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = redo_.rbegin(); it != redo_.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->Label());
    }
}

void DocumentBase::trimUndoIfNeeded() {
    if (undo_limit_ == 0) return;
    while (undo_.size() > undo_limit_) {
        // If we drop history older than the save marker,
        // we must shift the marker accordingly to preserve meaning.
        undo_.pop_front();
        if (saved_undo_size_ > 0) {
            saved_undo_size_ -= 1;
        }
    }
}

bool DocumentBase::save() {
    return asset_reg_.SaveAsset(guid_);
}

bool DocumentBase::saveAs(std::string_view) {
    return false;
}

}  // namespace cave
