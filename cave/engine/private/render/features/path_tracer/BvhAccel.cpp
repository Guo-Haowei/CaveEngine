#include "BvhAccel.h"

#include <algorithm>

namespace cave::render {

using namespace math;

namespace {

int DominantAxis(const AABB& aabb) {
    const Vec3f span = aabb.size();
    int axis = 0;
    if (span[axis] < span.y) {
        axis = 1;
    }
    if (span[axis] < span.z) {
        axis = 2;
    }

    return axis;
}

float SurfaceArea(const Box3& box) {
    if (!box.valid()) return 0.0f;
    Vec3f span = box.size();
    const float result = 2.0f * (span.x * span.y + span.x * span.z + span.y * span.z);
    return result;
}

}  // namespace

class BvhSorter;

class BvhBuilder {
public:
    BvhBuilder(std::span<const Vec3f> vertices, std::span<const Vec3i> triangles);

    Ref<BvhAccel> constructHelper(const BvhAccel* parent, std::span<uint32_t> indices) const;

    uint32_t getBvhCount() const { return m_bvh_counter; }

private:
    BvhBuilder(const BvhBuilder&) = delete;

    void splitByAxis(BvhAccel* parent, std::span<uint32_t> indices) const;
    AABB aabbFromTriangles(std::span<const uint32_t> indices) const;

    mutable uint32_t m_bvh_counter = 0;

    std::span<const Vec3f> m_vertices;
    std::span<const Vec3i> m_triangles;
    std::vector<AABB> m_aabbs;
    std::vector<Vec3f> m_centroids;

    friend class BvhSorter;
};

class BvhSorter {
public:
    BvhSorter(int axis, const BvhBuilder& builder)
        : m_axis(axis), m_builder(builder) {}

    bool operator()(uint32_t lhs, uint32_t rhs) const {
        auto aabb_1 = m_builder.m_aabbs.at(lhs);
        auto aabb_2 = m_builder.m_aabbs.at(rhs);
        Vec3f center_1 = aabb_1.center();
        Vec3f center_2 = aabb_2.center();
        return center_1[m_axis] < center_2[m_axis];
    }

private:
    const int m_axis;
    const BvhBuilder& m_builder;
};

BvhBuilder::BvhBuilder(std::span<const Vec3f> vertices,
                       std::span<const Vec3i> triangles)
    : m_vertices(vertices), m_triangles(triangles) {
    m_aabbs.resize(m_triangles.size());
    m_centroids.resize(m_triangles.size());
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        const auto& triangle = m_triangles[i];
        const Vec3f& a = m_vertices[triangle.x];
        const Vec3f& b = m_vertices[triangle.y];
        const Vec3f& c = m_vertices[triangle.z];
        m_centroids[i] = (1.0f / 3.0f) * (a + b + c);
        m_aabbs[i].invalidate();
        m_aabbs[i].expandToInclude(a);
        m_aabbs[i].expandToInclude(b);
        m_aabbs[i].expandToInclude(c);
        m_aabbs[i].expandIfDegenerate();
    }
}

AABB BvhBuilder::aabbFromTriangles(std::span<const uint32_t> indices) const {
    AABB aabb;
    for (uint32_t index : indices) {
        const auto& points = m_triangles[index];
        aabb.expandToInclude(m_vertices[points.x]);
        aabb.expandToInclude(m_vertices[points.y]);
        aabb.expandToInclude(m_vertices[points.z]);
    }
    aabb.expandIfDegenerate();
    return aabb;
}

void BvhBuilder::splitByAxis(BvhAccel* parent, std::span<uint32_t> indices) const {
    const int axis = DominantAxis(parent->m_aabb);
    BvhSorter sorter(axis, *this);
    std::sort(indices.begin(), indices.end(), sorter);
    const size_t mid = indices.size() / 2;
    std::span<uint32_t> left(indices.begin(), indices.begin() + mid);
    std::span<uint32_t> right(indices.begin() + mid, indices.end());

    parent->m_left = constructHelper(parent, left);
    parent->m_right = constructHelper(parent, right);
}

Ref<BvhAccel> BvhBuilder::constructHelper(const BvhAccel* parent, std::span<uint32_t> indices) const {
    const int depth = parent ? parent->m_depth + 1 : 0;
    if (depth > 64) {
        CRASH_NOW_MSG("TOO MANY LEVELS OF BVH");
        return nullptr;
    }

    const std::size_t triangle_count = indices.size();
    const AABB parent_aabb = aabbFromTriangles(indices);
    auto bvh = std::make_shared<BvhAccel>(m_bvh_counter++, parent);
    bvh->m_aabb = parent_aabb;
    bvh->m_depth = depth;

    if (triangle_count == 1) {
        bvh->m_is_leaf = true;
        bvh->m_triangle_index = indices[0];
        return bvh;
    }

    // @TODO: refactor
    const float parent_surface = SurfaceArea(parent_aabb);

    // @TODO rework
    if (triangle_count <= 4 || parent_surface == 0.0f) {
        DEV_ASSERT(parent_surface > 0.0f);
        splitByAxis(bvh.get(), indices);
        return bvh;
    }

    constexpr int BUCKED_MAX = 12;
    struct BucketInfo {
        int count = 0;
        AABB box;
    };
    std::array<BucketInfo, BUCKED_MAX> buckets;

    AABB centroidBox;
    for (const auto index : indices) {
        const Vec3f& point = m_centroids.at(index);
        centroidBox.expandToInclude(point);
    }
    centroidBox.expandIfDegenerate();

    const int axis = DominantAxis(centroidBox);
    const float tmin = centroidBox.min()[axis];
    const float tmax = centroidBox.max()[axis];

    for (int index : indices) {
        float tmp = ((m_centroids.at(index)[axis] - tmin) * BUCKED_MAX) / (tmax - tmin);
        int slot = static_cast<int>(tmp);
        slot = clamp(slot, 0, BUCKED_MAX - 1);
        BucketInfo& bucket = buckets[slot];
        ++bucket.count;
        bucket.box.expandToInclude(m_aabbs.at(index));
    }

    float costs[BUCKED_MAX - 1];
    for (int i = 0; i < BUCKED_MAX - 1; ++i) {
        AABB b0, b1;
        int count0 = 0, count1 = 0;
        for (int j = 0; j <= i; ++j) {
            b0.expandToInclude(buckets[j].box);
            count0 += buckets[j].count;
        }
        for (int j = i + 1; j < BUCKED_MAX; ++j) {
            b1.expandToInclude(buckets[j].box);
            count1 += buckets[j].count;
        }

        constexpr float travCost = 0.125f;
        costs[i] = travCost + (count0 * SurfaceArea(b0) + count1 * SurfaceArea(b1)) / parent_surface;
    }

    int splitIndex = 0;
    float minCost = costs[splitIndex];
    for (int i = 0; i < BUCKED_MAX - 1; ++i) {
        // printf("cost of split after bucket %d is %f\n", i, costs[i]);
        if (costs[i] < minCost) {
            splitIndex = i;
            minCost = costs[splitIndex];
        }
    }

    // printf("split index is %d\n", splitIndex);

    std::vector<uint32_t> leftPartition;
    std::vector<uint32_t> rightPartition;

    for (auto index : indices) {
        const auto& t = m_centroids.at(index);
        float tmp = (t[axis] - tmin) / (tmax - tmin);
        tmp *= BUCKED_MAX;
        int slot = static_cast<int>(tmp);
        slot = math::clamp(slot, 0, BUCKED_MAX - 1);
        if (slot <= splitIndex) {
            leftPartition.push_back(index);
        } else {
            rightPartition.push_back(index);
        }
    }

    bvh->m_left = constructHelper(bvh.get(), leftPartition);
    bvh->m_right = constructHelper(bvh.get(), rightPartition);
    return bvh;
}

void BvhAccel::discoverIdx() {
    m_miss_index = -1;

    // hit link (find right link)
    for (const BvhAccel* cursor = m_parent; cursor; cursor = cursor->m_parent) {
        if (cursor->m_right && cursor->m_right->m_index > m_index) {
            m_miss_index = cursor->m_right->m_index;
            break;
        }
    }

    // if it's a hit, then check the left child node
    // if there's not left child, treat it as a miss, so next bounding box to check is the miss node
    m_hit_index = m_left ? m_left->m_index : m_miss_index;
}

void BvhAccel::fillGpuBvhAccel(std::vector<GpuPtBvh>& out) {
    discoverIdx();
    DEV_ASSERT(m_aabb.valid());

    GpuPtBvh gpu_bvh;
    gpu_bvh.min = m_aabb.min();
    gpu_bvh.max = m_aabb.max();
    gpu_bvh.hit_idx = m_hit_index;
    gpu_bvh.miss_idx = m_miss_index;
    gpu_bvh.leaf = !!m_is_leaf;
    gpu_bvh.triangleIndex = -1;
    if (m_is_leaf) {
        DEV_ASSERT(m_triangle_index != -1);
        gpu_bvh.triangleIndex = m_triangle_index;
    }

    out.push_back(gpu_bvh);
    if (!m_is_leaf) {
        DEV_ASSERT(m_left && m_right);
    }
    if (m_left) {
        m_left->fillGpuBvhAccel(out);
    }
    if (m_right) {
        m_right->fillGpuBvhAccel(out);
    }
}

Ref<BvhAccel> BvhAccel::construct(std::span<const uint32_t> indices,
                                  std::span<const Vec3f> vertices) {

    const int index_count = (int)indices.size();
    DEV_ASSERT(index_count % 3 == 0);
    const int triangle_count = index_count / 3;

    std::vector<Vec3i> triangles(triangle_count);
    std::vector<uint32_t> triangle_indices(triangle_count);

    // @TODO: centroid?
    for (int i = 0; i < index_count; i += 3) {
        const int index = i / 3;

        const int a = indices[i];
        const int b = indices[i + 1];
        const int c = indices[i + 2];
        triangles[index] = Vec3i(a, b, c);
        triangle_indices[index] = index;
    }

    BvhBuilder builder(vertices, triangles);
    return builder.constructHelper(nullptr, triangle_indices);
}

}  // namespace cave::render
