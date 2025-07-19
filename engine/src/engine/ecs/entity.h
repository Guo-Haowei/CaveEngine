#pragma once

namespace cave {
class Archive;
}

namespace cave::ecs {

class Entity {
public:
    static constexpr uint32_t INVALID_ID = 0;
    static constexpr uint32_t MAX_ID = ~0u;

    explicit constexpr Entity()
        : m_id(INVALID_ID) {}

    explicit constexpr Entity(uint32_t p_handle)
        : m_id(p_handle) {}

    static constexpr Entity Null() { return Entity(); }

    ~Entity() = default;

    bool operator==(const Entity& p_rhs) const { return m_id == p_rhs.m_id; }

    bool operator!=(const Entity& p_rhs) const { return m_id != p_rhs.m_id; }

    bool IsValid() const { return m_id != INVALID_ID; }

    void MakeInvalid() { m_id = INVALID_ID; }

    constexpr uint32_t GetId() const { return m_id; }

private:
    uint32_t m_id;
};

}  // namespace cave::ecs

namespace std {

template<>
struct hash<cave::ecs::Entity> {
    std::size_t operator()(const cave::ecs::Entity& p_entity) const { return std::hash<uint32_t>{}(p_entity.GetId()); }
};

template<>
struct less<cave::ecs::Entity> {
    constexpr bool operator()(const cave::ecs::Entity& p_lhs, const cave::ecs::Entity& p_rhs) const {
        return p_lhs.GetId() < p_rhs.GetId();
    }
};

template<>
struct equal_to<cave::ecs::Entity> {
    constexpr bool operator()(const cave::ecs::Entity& p_lhs, const cave::ecs::Entity& p_rhs) const {
        return p_lhs.GetId() == p_rhs.GetId();
    }
};

}  // namespace std
