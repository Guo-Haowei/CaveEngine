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
