#pragma once
#include "allocator_interface.h"

namespace cave::memory {

class LinearAllocator : public IAllocator {
public:
    LinearAllocator(size_t p_capacity);
    ~LinearAllocator();

    void* Allocate(size_t p_size, size_t p_alignment = kDefaultAlignment) override;
    void Deallocate(void* p_ptr, size_t p_size) override;

    void Reset();

    std::size_t Capacity() const { return m_capacity; }

private:
    static std::uintptr_t PtrToInt(void* p) {
        return reinterpret_cast<std::uintptr_t>(p);
    }

    const size_t m_capacity;
    void* m_base = nullptr;
    size_t m_offset = 0;
};

}  // namespace cave::memory
