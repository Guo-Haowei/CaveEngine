#include "DocumentBase.h"

#include "cave/runtime/framework/EngineServices.h"

#include "engine/private/runtime/framework/AssetRegistry.h"

#define DEBUG_DOC IN_USE
#if USING(DEBUG_DOC)
#define DEBUG_DOC_LOG(...) LOG_TRACE(__VA_ARGS__)
#else
#define DEBUG_DOC_LOG(...) (void)0
#endif

namespace cave {

DocumentBase::DocumentBase(EngineServices& services, const Guid& guid)
    : asset_reg_(services.assetRegistry())
    , scene_reg_(services.sceneRegistry())
    , guid_(guid) {

    handle_ = asset_reg_.findByGuid(guid).unwrap();
    asset_ = handle_.wait();
}

bool DocumentBase::apply(std::unique_ptr<IEditCmd> cmd, uint32_t coalesce) {
    if (!cmd) return false;

    if (!undo_.empty() /*&& coalesce != 0 && last_coalesce_ == coalesce*/) {
        IEditCmd* last = undo_.back().get();
        if (last && last->canCoalesceWith(cmd.get())) {
            cmd->apply(*this);
            last->coalesceFrom(std::move(cmd));
            redo_.clear();
            touchDirtyAfterEdit();
            return true;
        }
    }

    cmd->apply(*this);

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

    cmd->undo(*this);
    redo_.push_back(std::move(cmd));

    last_coalesce_ = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

bool DocumentBase::redo() {
    if (redo_.empty()) return false;
    auto cmd = std::move(redo_.back());
    redo_.pop_back();

    cmd->apply(*this);
    undo_.push_back(std::move(cmd));

    last_coalesce_ = 0;
    recomputeDirtyAfterHistoryMove();
    return true;
}

void DocumentBase::undoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = undo_.rbegin(); it != undo_.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->label());
    }
}

void DocumentBase::redoLabels(std::vector<std::string>& out, int max_items) const {
    out.clear();
    int count = 0;
    for (auto it = redo_.rbegin(); it != redo_.rend() && count < max_items; ++it, ++count) {
        out.emplace_back((*it)->label());
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
    return asset_reg_.saveAsset(guid_);
}

bool DocumentBase::saveAs(std::string_view) {
    return false;
}

}  // namespace cave
