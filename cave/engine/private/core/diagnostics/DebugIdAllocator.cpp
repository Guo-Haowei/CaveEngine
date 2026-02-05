#include "DebugIdAllocator.h"

namespace cave::detail {

uint64_t GenUID() {
    static std::atomic<uint64_t> s_id;
    return s_id.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace cave::detail
