#include "HealthHUDController.h"

#include <format>

#include "cave/runtime/ecs/components/HierarchyComponent.h"
#include "cave/runtime/ui/UIComponents.h"

namespace super_cave_boy {

using namespace cave;

void HealthHUDController::start() {
    for (int i = 1; i <= 10; ++i) {
        auto name = std::format("Heart{}", i);
        auto ent = query().findChildByName(name, entity());
        if (ent.isNull()) {
            break;
        }

        m_images.push_back(ent);
    }
}

void HealthHUDController::update(float) {
    // @TODO: fetch it from session
    int n = 3;
    display(n);
}

void HealthHUDController::display(int n) {
    for (int i = 0; i < static_cast<int>(m_images.size()); ++i) {
        auto* hier = query().component<HierarchyComponent>(m_images[i]);
        if (DEV_VERIFY(hier)) {
            hier->local_visible = i < n;
        }
    }
}

}  // namespace super_cave_boy
