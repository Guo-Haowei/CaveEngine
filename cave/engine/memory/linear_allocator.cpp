#include "linear_allocator.h"

namespace cave::memory {

#define DEBUG_ALLOCATOR USE_IF(USING(DEBUG_BUILD))

LinearAllocator::LinearAllocator(size_t p_capacity)
    : m_capacity(p_capacity) {
    DEV_ASSERT(p_capacity);
    m_base = std::malloc(p_capacity);
    DEV_ASSERT_MSG(m_base, "malloc failed");
#if USING(DEBUG_ALLOCATOR)
    std::memset(m_base, 0xCD, m_capacity);
#endif
}

LinearAllocator::~LinearAllocator() {
    if (m_base) {
        std::free(m_base);
        m_base = nullptr;
    }
}

void* LinearAllocator::Allocate(size_t p_size, size_t p_alignment) {
    DEV_ASSERT(p_size);
    DEV_ASSERT(p_alignment && IsPow2(p_alignment));

    const size_t alloc_size = AlignUp(p_size, p_alignment);
    const size_t new_offset = m_offset + alloc_size;

    if (new_offset > m_capacity) {
        CRASH_NOW_MSG("failed to allocate");  // @TODO: add number and stuff
        return nullptr;
    }

    char* ptr = (char*)m_base + m_offset;
    m_offset += alloc_size;
    return ptr;
}

void LinearAllocator::Deallocate(void*, size_t) {
    // do nothing
}

void LinearAllocator::Reset() {
    m_offset = 0;
#if USING(DEBUG_ALLOCATOR)
    std::memset(m_base, 0xDD, m_capacity);
#endif
}

}  // namespace cave::memory
