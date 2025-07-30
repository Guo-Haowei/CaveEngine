#pragma once
#include "component_manager.h"

namespace cave::ecs {

template<ComponentType T>
void ComponentManager<T>::Reserve(size_t p_capacity) {
    if (p_capacity) {
        m_componentArray.reserve(p_capacity);
        m_entityArray.reserve(p_capacity);
        m_lookup.reserve(p_capacity);
    }
}

template<ComponentType T>
void ComponentManager<T>::Clear() {
    m_componentArray.clear();
    m_entityArray.clear();
    m_lookup.clear();
}

template<ComponentType T>
void ComponentManager<T>::Copy(const ComponentManager<T>& p_other) {
    Clear();
    m_componentArray = p_other.m_componentArray;
    m_entityArray = p_other.m_entityArray;
    m_lookup = p_other.m_lookup;
}

template<ComponentType T>
void ComponentManager<T>::Copy(const IComponentManager& p_other) {
    Copy((ComponentManager<T>&)p_other);
}

template<ComponentType T>
void ComponentManager<T>::Merge(ComponentManager<T>& p_other) {
    const size_t reserved = GetCount() + p_other.GetCount();
    m_componentArray.reserve(reserved);
    m_entityArray.reserve(reserved);
    m_lookup.reserve(reserved);

    for (size_t i = 0; i < p_other.GetCount(); ++i) {
        Entity entity = p_other.m_entityArray[i];
        DEV_ASSERT(!Contains(entity));
        m_entityArray.push_back(entity);
        m_lookup[entity] = m_componentArray.size();
        m_componentArray.push_back(std::move(p_other.m_componentArray[i]));
    }

    p_other.Clear();
}

template<ComponentType T>
void ComponentManager<T>::Merge(IComponentManager& p_other) {
    Merge((ComponentManager<T>&)p_other);
}

template<ComponentType T>
void ComponentManager<T>::Remove(const Entity& p_entity) {
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
bool ComponentManager<T>::Contains(const Entity& p_entity) const {
    if (m_lookup.empty()) {
        return false;
    }
    return m_lookup.find(p_entity) != m_lookup.end();
}

template<ComponentType T>
T& ComponentManager<T>::GetComponentByIndex(size_t p_index) {
    DEV_ASSERT(p_index < m_componentArray.size());
    return m_componentArray[p_index];
}

template<ComponentType T>
const T& ComponentManager<T>::GetComponentByIndex(size_t p_index) const {
    DEV_ASSERT(p_index < m_componentArray.size());
    return m_componentArray[p_index];
}

template<ComponentType T>
T* ComponentManager<T>::GetComponent(const Entity& p_entity) {
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
T& ComponentManager<T>::Create(const Entity& p_entity) {
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
