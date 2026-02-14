#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/IComponentPool.h"

namespace cave {

class Scene;
class SceneEdit;

}  // namespace cave

namespace cave::ecs {

template<ComponentType T>
class ComponentPool : public IComponentPool {
public:
    ComponentPool(size_t p_capacity = 0) { Reserve(p_capacity); }

    void Reserve(size_t p_capacity);

    void Clear() override;

    void Copy(const ComponentPool<T>& p_other);

    void Copy(const IComponentPool& p_other) override;

    void Merge(ComponentPool<T>&& p_other);

    void Merge(IComponentPool&& p_other) override;

    void Remove(const Entity& p_entity) override;

    bool Contains(const Entity& p_entity) const override;

    T& GetComponentByIndex(size_t p_index);

    const T& GetComponentByIndex(size_t p_index) const;

    T* GetComponent(const Entity& p_entity);

    size_t GetCount() const override { return m_componentArray.size(); }

    Option<size_t> FindIndex(Entity p_entity) const {
        auto it = m_lookup.find(p_entity);
        if (it == m_lookup.end()) return None();
        return Some(it->second);
    }

    T& Create(const Entity& p_entity);

    const std::vector<Entity>& GetEntityArray() const override {
        return m_entityArray;
    }

    void* GetRaw(Entity p_ent) override {
        return (void*)GetComponent(p_ent);
    }

    void* CreateDefaultRaw(Entity p_ent) override {
        T& c = Create(p_ent);
        return (void*)&c;
    }

protected:
    std::vector<T> m_componentArray;

    friend class ::cave::Scene;
};

class ComponentLibrary {
public:
    struct LibraryEntry {
        std::unique_ptr<IComponentPool> manager = nullptr;
        uint64_t version = 0;
    };

    template<ComponentType T>
    inline ComponentPool<T>& RegisterManager(const std::string& p_name, uint64_t p_version = 0) {
        DEV_ASSERT(m_entries.find(p_name) == m_entries.end());
        m_entries[p_name].manager = std::make_unique<ComponentPool<T>>();
        m_entries[p_name].version = p_version;
        return static_cast<ComponentPool<T>&>(*(m_entries[p_name].manager));
    }

private:
    std::unordered_map<std::string, LibraryEntry> m_entries;

    friend class ::cave::Scene;
    friend class ::cave::SceneEdit;
};

}  // namespace cave::ecs

// @TODO: refactor?
namespace cave::ecs {

template<ComponentType T>
void ComponentPool<T>::Reserve(size_t p_capacity) {
    if (p_capacity) {
        m_componentArray.reserve(p_capacity);
        m_entityArray.reserve(p_capacity);
        m_lookup.reserve(p_capacity);
    }
}

template<ComponentType T>
void ComponentPool<T>::Clear() {
    m_componentArray.clear();
    m_entityArray.clear();
    m_lookup.clear();
}

template<ComponentType T>
void ComponentPool<T>::Copy(const ComponentPool<T>& p_other) {
    Clear();
    m_componentArray = p_other.m_componentArray;
    m_entityArray = p_other.m_entityArray;
    m_lookup = p_other.m_lookup;
}

template<ComponentType T>
void ComponentPool<T>::Copy(const IComponentPool& p_other) {
    Copy((ComponentPool<T>&)p_other);
}

template<ComponentType T>
void ComponentPool<T>::Merge(ComponentPool<T>&& p_other) {
    const size_t base_count = GetCount();
    const size_t other_count = p_other.GetCount();
    const size_t reserved = base_count + other_count;
    m_componentArray.reserve(reserved);
    m_entityArray.reserve(reserved);
    m_lookup.reserve(reserved);

    for (size_t i = 0; i < other_count; ++i) {
        Entity entity = p_other.m_entityArray[i];
        DEV_ASSERT(!Contains(entity));
        m_entityArray.push_back(entity);
        m_lookup[entity] = base_count + i;
        m_componentArray.push_back(std::move(p_other.m_componentArray[i]));
    }

    p_other.Clear();
}

template<ComponentType T>
void ComponentPool<T>::Merge(IComponentPool&& p_other) {
    Merge((ComponentPool<T>&&)p_other);
}

template<ComponentType T>
void ComponentPool<T>::Remove(const Entity& p_entity) {
    auto it = m_lookup.find(p_entity);
    if (it == m_lookup.end()) {
        return;
    }

    const size_t index = it->second;
    DEV_ASSERT_INDEX(index, m_entityArray.size());
    const size_t last = m_componentArray.size() - 1;

    if (index != last) {
        // 1) Move last component into the gap
        m_componentArray[index] = std::move(m_componentArray[last]);

        // 2) Move last entity id into the gap
        const Entity movedEntity = m_entityArray[last];
        m_entityArray[index] = movedEntity;

        // 3) Fix the moved entity's index in the lookup
        m_lookup[movedEntity] = index;
    }

    // 4) Pop the last slot and erase the removed entity from the map
    m_componentArray.pop_back();
    m_entityArray.pop_back();
    m_lookup.erase(it);
}

template<ComponentType T>
bool ComponentPool<T>::Contains(const Entity& p_entity) const {
    if (m_lookup.empty()) {
        return false;
    }
    return m_lookup.find(p_entity) != m_lookup.end();
}

template<ComponentType T>
T& ComponentPool<T>::GetComponentByIndex(size_t p_index) {
    DEV_ASSERT(p_index < m_componentArray.size());
    return m_componentArray[p_index];
}

template<ComponentType T>
const T& ComponentPool<T>::GetComponentByIndex(size_t p_index) const {
    DEV_ASSERT(p_index < m_componentArray.size());
    return m_componentArray[p_index];
}

template<ComponentType T>
T* ComponentPool<T>::GetComponent(const Entity& p_entity) {
    if (!p_entity.IsValid() || m_lookup.empty()) {
        return nullptr;
    }

    auto it = m_lookup.find(p_entity);

    if (it == m_lookup.end()) {
        return nullptr;
    }

    return &m_componentArray[it->second];
}

template<ComponentType T>
T& ComponentPool<T>::Create(const Entity& p_entity) {
    DEV_ASSERT(p_entity.IsValid());

    const size_t componentCount = m_componentArray.size();
    DEV_ASSERT(m_lookup.find(p_entity) == m_lookup.end());
    DEV_ASSERT(m_entityArray.size() == componentCount);
    DEV_ASSERT(m_lookup.size() == componentCount);

    m_lookup[p_entity] = componentCount;
    m_componentArray.emplace_back();
    m_entityArray.push_back(p_entity);
    return m_componentArray.back();
}

}  // namespace cave::ecs
