#pragma once
#include "cave/core/math/AABB.h"
#include "cave/core/math/Vec.h"

namespace cave {
#include "shader_defines.slang.h"
#include "path_tracer_defines.slang.h"
}  // namespace cave

namespace cave::render {

class BvhAccel {
public:
    BvhAccel(int index,
             const BvhAccel* parent)
        : m_index(index), m_parent(parent) {}

    void fillGpuBvhAccel(std::vector<GpuPtBvh>& out);

    static Ref<BvhAccel> construct(std::span<const uint32_t> indices,
                                   std::span<const math::Vec3f> vertices);

private:
    void discoverIdx();

    const int m_index;
    const BvhAccel* m_parent;

    math::AABB m_aabb;

    int m_triangle_index = -1;
    int m_hit_index = -1;
    int m_miss_index = -1;
    int m_depth = 0;
    Ref<BvhAccel> m_left = nullptr;
    Ref<BvhAccel> m_right = nullptr;
    bool m_is_leaf = false;

    friend class BvhBuilder;
};

}  // namespace cave::render
