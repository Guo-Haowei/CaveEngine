#include "engine/memory/linear_allocator.h"

namespace cave::memory {

static inline bool IsAligned(const void* p_ptr, size_t p_alignment) {
    return (reinterpret_cast<std::uintptr_t>(p_ptr) & (p_alignment - 1)) == 0;
}

TEST(linear_allocator, construct) {
    constexpr size_t cap = 1024;
    LinearAllocator alloc(cap);
    EXPECT_EQ(alloc.Capacity(), cap);
    EXPECT_EQ(alloc.Used(), 0);
}

TEST(linear_allocator, simple_alloc_default_alignment) {
    constexpr size_t cap = 1024;
    LinearAllocator alloc(cap);

    void* p1 = alloc.Allocate(24);
    EXPECT_NE(p1, nullptr);
    EXPECT_TRUE(IsAligned(p1, IAllocator::kDefaultAlignment));
    EXPECT_EQ(alloc.Used() >= 24u, true);

    void* p2 = alloc.Allocate(40, 16);
    EXPECT_NE(p2, nullptr);
    EXPECT_TRUE(IsAligned(p2, IAllocator::kDefaultAlignment));
    EXPECT_TRUE(p2 > p1);
}

TEST(linear_allocator, reset_reuses_memory) {
    constexpr size_t cap = 1024;
    LinearAllocator alloc(cap);

    void* p1 = alloc.Allocate(128);
    void* p2 = alloc.Allocate(256);
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
    EXPECT_TRUE(p2 > p1);
    const size_t usedBefore = alloc.Used();
    EXPECT_TRUE(usedBefore > 0);

    alloc.Reset();
    EXPECT_EQ(alloc.Used(), 0);

    void* p3 = alloc.Allocate(128);
    EXPECT_TRUE(IsAligned(p3, IAllocator::kDefaultAlignment));
    EXPECT_EQ(p3, p1);
}

TEST(linear_allocator, stress_many_small) {
    const size_t cap = 64 * 1024;
    LinearAllocator alloc(cap);

    const size_t count = 1000;
    const size_t each = 32;
    for (size_t i = 0; i < count; ++i) {
        void* p = alloc.Allocate(each);
        if (p == nullptr) {
            break;
        }
        EXPECT_TRUE(IsAligned(p, IAllocator::kDefaultAlignment));
    }
    EXPECT_TRUE(alloc.Used() <= alloc.Capacity());
}

TEST(linear_allocator, boundary_no_overflow) {
    const size_t cap = 256;
    LinearAllocator alloc(cap);

    void* p1 = alloc.Allocate(192);
    EXPECT_NE(p1, nullptr);

    void* p2 = alloc.Allocate(64);
    EXPECT_NE(p2, nullptr);
    EXPECT_EQ(alloc.Used(), cap);

    void* p3 = alloc.Allocate(1, 16);
    EXPECT_EQ(p3, nullptr);
}

TEST(linear_allocator, debug_patterns) {
    if constexpr (LinearAllocator::kDebug) {
        const size_t cap = 1024;
        LinearAllocator alloc(cap);

        const size_t N = 128;
        uint8_t* p = static_cast<uint8_t*>(alloc.Allocate(N, 16));
        EXPECT_NE(p, nullptr);
        for (size_t i = 0; i < N; ++i) {
            EXPECT_EQ(p[i], 0xCD);
        }

        ::memset(p, 0xAB, N);

        const size_t used = alloc.Used();
        alloc.Reset();
        uint8_t* base = static_cast<uint8_t*>(alloc.Allocate(used, 1));
        for (std::size_t i = 0; i < used; ++i) {
            EXPECT_EQ(base[i], 0xDD);
        }
    }
}

#if 0

#if defined(DEV_DEBUG_CLEAR_LINEAR_ALLOCATOR) || defined(DEV_DEBUG_INIT_LINEAR_ALLOCATOR)
// Optional debug pattern tests; only meaningful if you compiled with your debug memset flags.

static void Test_Debug_Patterns() {
}
#endif

#endif

}  // namespace cave::memory
