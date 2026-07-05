#include "SceneOwner.h"

namespace cave {

void SceneOwner::requestSceneChange(std::string path) {
    m_pending_change = Some(std::move(path));
}

void SceneOwner::requestSceneReload() {
    m_pending_reload = true;
}

void SceneOwner::flushSceneCommands() {
    if (m_pending_reload) {
        m_pending_reload = false;
        commitSceneReload();
    }
    if (m_pending_change.is_some()) {
        std::string path = m_pending_change.unwrap_unchecked();
        m_pending_change = None();
        if (!path.empty()) {
            commitSceneChange(std::move(path));
        }
    }
}

}  // namespace cave
