#include "AssetEntry.h"

namespace cave {

AssetRef AssetEntry::wait() {
    std::unique_lock lock(mutex_);

    cv_.wait(lock, [this]() {
        return status == AssetStatus::Loaded || status == AssetStatus::Failed;
    });

    if (status == AssetStatus::Failed) {
        return nullptr;
    }

    return asset;
}

void AssetEntry::markLoaded(AssetRef asset_ref) {
    mutex_.lock();
    asset = std::move(asset_ref);
    status = AssetStatus::Loaded;
    mutex_.unlock();
    cv_.notify_all();
}

void AssetEntry::markFailed() {
    mutex_.lock();
    status = AssetStatus::Failed;
    mutex_.unlock();
    cv_.notify_all();
}

}  // namespace cave
