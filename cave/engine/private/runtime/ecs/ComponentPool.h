#pragma once
#include "cave/runtime/ecs/Entity.h"
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
    std::unique_ptr<IComponentPool> clone() const override {
        std::unique_ptr<IComponentPool> clone = std::make_unique<ComponentPool<T>>();
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

    size_t count() const override { return component_array_.size(); }

    Option<size_t> findIndex(Entity ent) const {
        auto it = lookup_.find(ent);
        if (it == lookup_.end()) return None();
        return Some(it->second);
    }

    std::vector<T>& componentArray() {
        return component_array_;
    }

protected:
    std::vector<T> component_array_;

    friend class ::cave::Scene;
};

template<ComponentType T>
void ComponentPool<T>::reserve(size_t p_capacity) {
    if (p_capacity) {
        component_array_.reserve(p_capacity);
        entity_array_.reserve(p_capacity);
        lookup_.reserve(p_capacity);
    }
}

template<ComponentType T>
void ComponentPool<T>::clear() {
    component_array_.clear();
    entity_array_.clear();
    lookup_.clear();
}

template<ComponentType T>
void ComponentPool<T>::copy(const ComponentPool<T>& p_other) {
    clear();
    component_array_ = p_other.component_array_;
    entity_array_ = p_other.entity_array_;
    lookup_ = p_other.lookup_;
}

template<ComponentType T>
void ComponentPool<T>::copy(const IComponentPool& p_other) {
    copy((ComponentPool<T>&)p_other);
}

template<ComponentType T>
void ComponentPool<T>::merge(ComponentPool<T>&& p_other) {
    const size_t base_count = count();
    const size_t other_count = p_other.count();
    const size_t reserved = base_count + other_count;
    component_array_.reserve(reserved);
    entity_array_.reserve(reserved);
    lookup_.reserve(reserved);

    for (size_t i = 0; i < other_count; ++i) {
        Entity entity = p_other.entity_array_[i];
        DEV_ASSERT(!has(entity));
        entity_array_.push_back(entity);
        lookup_[entity] = base_count + i;
        component_array_.push_back(std::move(p_other.component_array_[i]));
    }

    p_other.clear();
}

template<ComponentType T>
void ComponentPool<T>::merge(IComponentPool&& p_other) {
    merge((ComponentPool<T>&&)p_other);
}

template<ComponentType T>
T& ComponentPool<T>::create(Entity p_ent) {
    DEV_ASSERT(p_ent.IsValid());

    const size_t componentCount = component_array_.size();
    DEV_ASSERT(lookup_.find(p_ent) == lookup_.end());
    DEV_ASSERT(entity_array_.size() == componentCount);
    DEV_ASSERT(lookup_.size() == componentCount);

    lookup_[p_ent] = componentCount;
    component_array_.emplace_back();
    entity_array_.push_back(p_ent);
    return component_array_.back();
}

template<ComponentType T>
void* ComponentPool<T>::createRaw(Entity p_ent) {
    T& c = create(p_ent);
    return (void*)&c;
}

template<ComponentType T>
void* ComponentPool<T>::getRaw(Entity p_ent) {
    return (void*)this->getComponent(p_ent);
}

template<ComponentType T>
const void* ComponentPool<T>::getRaw(Entity p_ent) const {
    return (const void*)this->getComponent(p_ent);
}

template<ComponentType T>
void ComponentPool<T>::remove(Entity p_ent) {
    auto it = lookup_.find(p_ent);
    if (it == lookup_.end()) {
        return;
    }

    const size_t index = it->second;
    DEV_ASSERT_INDEX(index, entity_array_.size());
    const size_t last = component_array_.size() - 1;

    if (index != last) {
        // 1) Move last component into the gap
        component_array_[index] = std::move(component_array_[last]);

        // 2) Move last entity id into the gap
        const Entity movedEntity = entity_array_[last];
        entity_array_[index] = movedEntity;

        // 3) Fix the moved entity's index in the lookup
        lookup_[movedEntity] = index;
    }

    // 4) Pop the last slot and erase the removed entity from the map
    component_array_.pop_back();
    entity_array_.pop_back();
    lookup_.erase(it);
}

template<ComponentType T>
T& ComponentPool<T>::getComponentByIndex(size_t p_index) {
    DEV_ASSERT(p_index < component_array_.size());
    return component_array_[p_index];
}

template<ComponentType T>
const T& ComponentPool<T>::getComponentByIndex(size_t p_index) const {
    DEV_ASSERT(p_index < component_array_.size());
    return component_array_[p_index];
}

template<ComponentType T>
T* ComponentPool<T>::getComponent(Entity p_ent) {
    if (!p_ent.IsValid() || lookup_.empty()) {
        return nullptr;
    }

    auto it = lookup_.find(p_ent);

    if (it == lookup_.end()) {
        return nullptr;
    }

    return &component_array_[it->second];
}

template<ComponentType T>
const T* ComponentPool<T>::getComponent(Entity p_ent) const {
    if (!p_ent.IsValid() || lookup_.empty()) {
        return nullptr;
    }

    auto it = lookup_.find(p_ent);

    if (it == lookup_.end()) {
        return nullptr;
    }

    return &component_array_[it->second];
}

}  // namespace cave::ecs
