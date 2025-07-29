#include "page_allocator.h"

namespace cave {

PageAllocator::PageAllocator(size_t totalSize)
    : m_total_size(totalSize), m_page_size(PlatformVM::PageSize()) {
    m_base = PlatformVM::Reserve(nullptr, m_totalSize);
    assert(m_base && "Virtual memory reservation failed");
}

PageAllocator::~PageAllocator() {
    if (m_base) {
        PlatformVM::Release(m_base, m_totalSize);
    }
}

void* PageAllocator::GetPtr(size_t offset) {
    assert(offset < m_totalSize);
    return static_cast<uint8_t*>(m_base) + offset;
}

void PageAllocator::Commit(size_t offset, size_t size, PageAccess access) {
    void* ptr = static_cast<uint8_t*>(m_base) + offset;
    PlatformVM::Commit(ptr, size, access);
}

void PageAllocator::Protect(size_t offset, size_t size, PageAccess access) {
    void* ptr = static_cast<uint8_t*>(m_base) + offset;
    PlatformVM::Protect(ptr, size, access);
}

void PageAllocator::Decommit(size_t offset, size_t size) {
    void* ptr = static_cast<uint8_t*>(m_base) + offset;
    PlatformVM::Decommit(ptr, size);
}

void PageAllocator::Release() {
    if (m_base) {
        PlatformVM::Release(m_base, m_totalSize);
        m_base = nullptr;
    }
}

void* PageAllocator::Allocate(size_t p_size, size_t p_alignment) {
    return nullptr;
}

void PageAllocator::Deallocate(void* p_ptr, size_t p_size) {
}

}  // namespace cave
