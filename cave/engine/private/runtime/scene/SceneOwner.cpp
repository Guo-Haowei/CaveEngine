#include "SceneOwner.h"

namespace cave {

void SceneOwner::requestSceneChange(std::string path) {
    pending_change_ = Some(std::move(path));
}

void SceneOwner::requestSceneReload() {
    pending_reload_ = true;
}

void SceneOwner::flushSceneCommands() {
    if (pending_reload_) {
        pending_reload_ = false;
        commitSceneReload();
    }
    if (pending_change_.is_some()) {
        std::string path = pending_change_.unwrap_unchecked();
        pending_change_ = None();
        if (!path.empty()) {
            commitSceneChange(std::move(path));
        }
    }
}

}  // namespace cave
