#include "cave/runtime/framework/IService.h"

namespace cave {

auto IService::Initialize() -> Result<void> {
    if (DEV_VERIFY(!m_initialized)) {
        auto res = InitializeImpl();
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        m_initialized = true;
    }

    return Result<void>();
}

void IService::Finalize() {
    if (m_initialized) {
        FinalizeImpl();

        m_initialized = false;
    }
}

}  // namespace cave
