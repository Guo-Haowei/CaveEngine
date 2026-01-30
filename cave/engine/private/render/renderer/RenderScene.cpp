#include "RenderScene.h"

namespace cave::render {

void RenderScene::Clear() {
    m_renderables.clear();
    m_meshes.clear();
    m_sprites.clear();
    ClearDirtyLists();
}

void RenderScene::ClearDirtyLists() {
}

}  // namespace cave::render
