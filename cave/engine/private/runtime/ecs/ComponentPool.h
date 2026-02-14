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

    std::unique_ptr<IComponentPool> Clone() const override {
        std::unique_ptr<IComponentPool> clone = std::make_unique<ComponentPool<T>>();
        clone->Copy(*this);
        return clone;
    }

    void Copy(const ComponentPool<T>& p_other);

    void Copy(const IComponentPool& p_other) override;

    void Merge(ComponentPool<T>&& p_other);

    void Merge(IComponentPool&& p_other) override;

    void Remove(const Entity& p_ent) override;

    T& GetComponentByIndex(size_t p_index);

    const T& GetComponentByIndex(size_t p_index) const;

    T* GetComponent(const Entity& p_ent);

    const T* GetComponent(const Entity& p_ent) const;

    size_t GetCount() const override { return m_component_array.size(); }

    Option<size_t> FindIndex(Entity p_ent) const {
        auto it = m_lookup.find(p_ent);
        if (it == m_lookup.end()) return None();
        return Some(it->second);
    }

    T& Create(const Entity& p_ent);

    void* GetRaw(Entity p_ent) override {
        return (void*)GetComponent(p_ent);
    }

    void* CreateDefaultRaw(Entity p_ent) override {
        T& c = Create(p_ent);
        return (void*)&c;
    }

    std::vector<T>& GetComponentArray() {
        return m_component_array;
    }

protected:
    std::vector<T> m_component_array;

    friend class ::cave::Scene;
};

template<ComponentType T>
void ComponentPool<T>::Reserve(size_t p_capacity) {
    if (p_capacity) {
        m_component_array.reserve(p_capacity);
        m_entity_array.reserve(p_capacity);
        m_lookup.reserve(p_capacity);
    }
}

template<ComponentType T>
void ComponentPool<T>::Clear() {
    m_component_array.clear();
    m_entity_array.clear();
    m_lookup.clear();
}

template<ComponentType T>
void ComponentPool<T>::Copy(const ComponentPool<T>& p_other) {
    Clear();
    m_component_array = p_other.m_component_array;
    m_entity_array = p_other.m_entity_array;
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
    m_component_array.reserve(reserved);
    m_entity_array.reserve(reserved);
    m_lookup.reserve(reserved);

    for (size_t i = 0; i < other_count; ++i) {
        Entity entity = p_other.m_entity_array[i];
        DEV_ASSERT(!Has(entity));
        m_entity_array.push_back(entity);
        m_lookup[entity] = base_count + i;
        m_component_array.push_back(std::move(p_other.m_component_array[i]));
    }

    p_other.Clear();
}

template<ComponentType T>
void ComponentPool<T>::Merge(IComponentPool&& p_other) {
    Merge((ComponentPool<T>&&)p_other);
}

template<ComponentType T>
void ComponentPool<T>::Remove(const Entity& p_ent) {
    auto it = m_lookup.find(p_ent);
    if (it == m_lookup.end()) {
        return;
    }

    const size_t index = it->second;
    DEV_ASSERT_INDEX(index, m_entity_array.size());
    const size_t last = m_component_array.size() - 1;

    if (index != last) {
        // 1) Move last component into the gap
        m_component_array[index] = std::move(m_component_array[last]);

        // 2) Move last entity id into the gap
        const Entity movedEntity = m_entity_array[last];
        m_entity_array[index] = movedEntity;

        // 3) Fix the moved entity's index in the lookup
        m_lookup[movedEntity] = index;
    }

    // 4) Pop the last slot and erase the removed entity from the map
    m_component_array.pop_back();
    m_entity_array.pop_back();
    m_lookup.erase(it);
}

template<ComponentType T>
T& ComponentPool<T>::GetComponentByIndex(size_t p_index) {
    DEV_ASSERT(p_index < m_component_array.size());
    return m_component_array[p_index];
}

template<ComponentType T>
const T& ComponentPool<T>::GetComponentByIndex(size_t p_index) const {
    DEV_ASSERT(p_index < m_component_array.size());
    return m_component_array[p_index];
}

template<ComponentType T>
T* ComponentPool<T>::GetComponent(const Entity& p_ent) {
    if (!p_ent.IsValid() || m_lookup.empty()) {
        return nullptr;
    }

    auto it = m_lookup.find(p_ent);

    if (it == m_lookup.end()) {
        return nullptr;
    }

    return &m_component_array[it->second];
}

template<ComponentType T>
const T* ComponentPool<T>::GetComponent(const Entity& p_ent) const {
    if (!p_ent.IsValid() || m_lookup.empty()) {
        return nullptr;
    }

    auto it = m_lookup.find(p_ent);

    if (it == m_lookup.end()) {
        return nullptr;
    }

    return &m_component_array[it->second];
}

template<ComponentType T>
T& ComponentPool<T>::Create(const Entity& p_ent) {
    DEV_ASSERT(p_ent.IsValid());

    const size_t componentCount = m_component_array.size();
    DEV_ASSERT(m_lookup.find(p_ent) == m_lookup.end());
    DEV_ASSERT(m_entity_array.size() == componentCount);
    DEV_ASSERT(m_lookup.size() == componentCount);

    m_lookup[p_ent] = componentCount;
    m_component_array.emplace_back();
    m_entity_array.push_back(p_ent);
    return m_component_array.back();
}

}  // namespace cave::ecs
