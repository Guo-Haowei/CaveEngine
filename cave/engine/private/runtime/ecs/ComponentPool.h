#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/IComponentPool.h"

namespace cave {

class Scene;
class SceneCommandExecutor;

}  // namespace cave

namespace cave::ecs {

template<ComponentType T>
class ComponentPool : public IComponentPool {
public:
    ComponentPool(size_t capacity = 0) { reserve(capacity); }

    void reserve(size_t capacity);

    void clear() override;

    // @TODO: refactor
    Owner<IComponentPool> clone() const override {
        Owner<IComponentPool> clone = std::make_unique<ComponentPool<T>>();
        clone->copy(*this);
        return clone;
    }

    void copy(const ComponentPool<T>& other);
    void copy(const IComponentPool& other) override;

    void merge(ComponentPool<T>&& other);
    void merge(IComponentPool&& other) override;

    T& create(Entity ent);
    void* createRaw(Entity ent) override;
    void remove(Entity ent) override;

    void* getRaw(Entity ent) override;
    const void* getRaw(Entity ent) const override;

    T* getComponent(Entity ent);

    const T* getComponent(Entity ent) const;

    T& getComponentByIndex(size_t index);
    const T& getComponentByIndex(size_t index) const;

    size_t count() const override { return m_component_array.size(); }

    Option<size_t> findIndex(Entity ent) const {
        auto it = m_lookup.find(ent);
        if (it == m_lookup.end()) return None();
        return Some(it->second);
    }

    Vector<T>& componentArray() {
        return m_component_array;
    }

protected:
    Vector<T> m_component_array;

    friend class ::cave::Scene;
};

template<ComponentType T>
void ComponentPool<T>::reserve(size_t p_capacity) {
    if (p_capacity) {
        m_component_array.reserve(p_capacity);
        m_entity_array.reserve(p_capacity);
        m_lookup.reserve(p_capacity);
    }
}

template<ComponentType T>
void ComponentPool<T>::clear() {
    m_component_array.clear();
    m_entity_array.clear();
    m_lookup.clear();
}

template<ComponentType T>
void ComponentPool<T>::copy(const ComponentPool<T>& p_other) {
    clear();
    m_component_array = p_other.m_component_array;
    m_entity_array = p_other.m_entity_array;
    m_lookup = p_other.m_lookup;
}

template<ComponentType T>
void ComponentPool<T>::copy(const IComponentPool& other) {
    copy((ComponentPool<T>&)other);
}

template<ComponentType T>
void ComponentPool<T>::merge(ComponentPool<T>&& other) {
    const size_t base_count = count();
    const size_t other_count = other.count();
    const size_t reserved = base_count + other_count;
    m_component_array.reserve(reserved);
    m_entity_array.reserve(reserved);
    m_lookup.reserve(reserved);

    for (size_t i = 0; i < other_count; ++i) {
        Entity entity = other.m_entity_array[i];
        DEV_ASSERT(!has(entity));
        m_entity_array.push_back(entity);
        m_lookup[entity] = base_count + i;
        m_component_array.push_back(std::move(other.m_component_array[i]));
    }

    other.clear();
}

template<ComponentType T>
void ComponentPool<T>::merge(IComponentPool&& other) {
    merge((ComponentPool<T>&&)other);
}

template<ComponentType T>
T& ComponentPool<T>::create(Entity ent) {
    DEV_ASSERT(ent.valid());

    const size_t componentCount = m_component_array.size();
    DEV_ASSERT(m_lookup.find(ent) == m_lookup.end());
    DEV_ASSERT(m_entity_array.size() == componentCount);
    DEV_ASSERT(m_lookup.size() == componentCount);

    m_lookup[ent] = componentCount;
    m_component_array.emplace_back();
    m_entity_array.push_back(ent);
    return m_component_array.back();
}

template<ComponentType T>
void* ComponentPool<T>::createRaw(Entity ent) {
    T& c = create(ent);
    return (void*)&c;
}

template<ComponentType T>
void* ComponentPool<T>::getRaw(Entity ent) {
    return (void*)this->getComponent(ent);
}

template<ComponentType T>
const void* ComponentPool<T>::getRaw(Entity ent) const {
    return (const void*)this->getComponent(ent);
}

template<ComponentType T>
void ComponentPool<T>::remove(Entity ent) {
    auto it = m_lookup.find(ent);
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
T& ComponentPool<T>::getComponentByIndex(size_t index) {
    DEV_ASSERT(index < m_component_array.size());
    return m_component_array[index];
}

template<ComponentType T>
const T& ComponentPool<T>::getComponentByIndex(size_t index) const {
    DEV_ASSERT(index < m_component_array.size());
    return m_component_array[index];
}

template<ComponentType T>
T* ComponentPool<T>::getComponent(Entity ent) {
    if (!ent.valid() || m_lookup.empty()) {
        return nullptr;
    }

    auto it = m_lookup.find(ent);

    if (it == m_lookup.end()) {
        return nullptr;
    }

    return &m_component_array[it->second];
}

template<ComponentType T>
const T* ComponentPool<T>::getComponent(Entity ent) const {
    if (!ent.valid() || m_lookup.empty()) {
        return nullptr;
    }

    auto it = m_lookup.find(ent);

    if (it == m_lookup.end()) {
        return nullptr;
    }

    return &m_component_array[it->second];
}

}  // namespace cave::ecs
