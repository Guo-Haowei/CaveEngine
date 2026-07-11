// =============================================================================
// File: cave/runtime/scene/SystemManager.h
// =============================================================================
#pragma once
#include <array>

#include "cave/core/base/NonCopyable.h"
#include "cave/runtime/scene/ISceneSystem.h"

namespace cave {

struct SceneContext;

class SystemManager : public NonCopyable {
public:
    SystemManager();
    ~SystemManager();

    template<SceneSystem T, typename... Args>
    T& add(Args&&... args) {
        static_assert(std::is_base_of_v<ISceneSystem, T>);
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = system.get();
        addImpl(std::move(system));
        return *raw;
    }

    template<SceneSystem T>
    T* get() {
        static_assert(std::is_base_of_v<ISceneSystem, T>);
        return static_cast<T*>(get(T::kSystemId));
    }

    template<SceneSystem T>
    const T* get() const {
        static_assert(std::is_base_of_v<ISceneSystem, T>);
        return static_cast<const T*>(get(T::kSystemId));
    }

    ISceneSystem* get(SceneSystemId id);
    const ISceneSystem* get(SceneSystemId id) const;

    template<SceneSystem T>
    bool has() const {
        return get<T>() != nullptr;
    }

    bool has(SceneSystemId id) const {
        return get(id) != nullptr;
    }

    void start(SceneContext& ctx);
    void shutdown();

    void update(SceneTickContext& ctx);

private:
    void addImpl(Owner<ISceneSystem>&& system);

private:
    Vector<Owner<ISceneSystem>> m_systems;
    std::array<ISceneSystem*, std::to_underlying(SceneSystemId::Count)> m_lookup{};

    bool m_scene_created{ false };
};

}  // namespace cave
