// =============================================================================
// File: cave/core/ids/Entity.h
// =============================================================================
#pragma once
#include <compare>
#include <cstdint>

namespace cave::ecs {

class Entity {
public:
    enum : uint32_t {
        kInvalidId = 0u,
        kMaxId = ~0u,
    };

    explicit constexpr Entity()
        : m_id(kInvalidId) {}

    explicit constexpr Entity(uint32_t handle)
        : m_id(handle) {}

    static constexpr Entity null() { return Entity(); }

    ~Entity() = default;

    std::strong_ordering operator<=>(const Entity&) const = default;

    bool valid() const { return m_id != kInvalidId; }
    bool isNull() const { return m_id == kInvalidId; }

    constexpr uint32_t id() const { return m_id; }

private:
    uint32_t m_id;
};

}  // namespace cave::ecs

namespace std {

template<>
struct hash<cave::ecs::Entity> {
    std::size_t operator()(const cave::ecs::Entity& ent) const {
        return std::hash<uint32_t>{}(ent.id());
    }
};

}  // namespace std
