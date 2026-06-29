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

    void onSceneCreate(SceneContext& ctx);
    void onSceneDestroy(SceneContext& ctx);

    void fixedUpdate(SceneTickContext& ctx);
    void update(SceneTickContext& ctx);
    void lateUpdate(SceneTickContext& ctx);

private:
    void addImpl(std::unique_ptr<ISceneSystem>&& system);

private:
    std::vector<std::unique_ptr<ISceneSystem>> systems_;
    std::array<ISceneSystem*, std::to_underlying(SceneSystemId::Count)> lookup_{};

    bool scene_created_{ false };
};

}  // namespace cave
