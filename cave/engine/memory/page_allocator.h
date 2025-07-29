#pragma once
#include "allocator_interface.h"

namespace cave {

class PageAllocator : public IAllocator {
public:
    PageAllocator(size_t p_total_size);
    ~PageAllocator();

    void* Allocate(size_t p_size, size_t p_alignment = 16) override;
    void Deallocate(void* p_ptr, size_t p_size) override;

    void* GetPtr(size_t p_offset);

    // void Commit(size_t offset, size_t size, PageAccess access = PageAccess::ReadWrite);
    // void Protect(size_t offset, size_t size, PageAccess access);
    // void Decommit(size_t offset, size_t size);
    // void Release();  // Optional explicit release

    size_t PageSize() const { return m_page_size; }
    size_t Capacity() const { return m_total_size; }

private:
    void* m_base = nullptr;
    size_t m_total_size = 0;
    size_t m_page_size = 0;
};

}  // namespace cave
