#pragma once

namespace cave {

class IAllocator {
public:
    virtual ~IAllocator() = default;

    virtual void* Allocate(size_t p_size, size_t p_alignment = 16) = 0;
    virtual void Deallocate(void* p_ptr, size_t p_size) = 0;
};

}  // namespace cave
