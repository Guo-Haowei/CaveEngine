#pragma once

namespace cave::memory {

FORCE_INLINE constexpr bool IsPow2(std::size_t x) {
    return x && ((x & (x - 1)) == 0);
}

FORCE_INLINE constexpr size_t AlignUp(size_t x, size_t a) {
    return (x + (a - 1)) & ~(a - 1);
}

class IAllocator {
public:
    static inline constexpr size_t kDefaultAlignment = 16;

    virtual ~IAllocator() = default;

    virtual void* Allocate(size_t p_size, size_t p_alignment = kDefaultAlignment) = 0;
    virtual void Deallocate(void* p_ptr, size_t p_size) = 0;
};

}  // namespace cave::memory
