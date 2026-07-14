#include "Scene.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/threading/JobSystem.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/core/io/archive.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/systems/AnimationSystem.h"
#include "engine/private/systems/EcsSystems.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

Scene::Scene(ecs::ComponentRegistry& reg) noexcept
    : m_component_registry(reg) {
}

Scene::Scene() noexcept
    : Scene(engine::GetComponentRegistry()) {
}

Scene::~Scene() = default;

void Scene::update(float dt) {
    CAVE_PROFILE_EVENT();

    dirtyFlags_.store(0);

    jobsystem::Context ctx;
    // animation
    RunTransformAnimationSystem(*this, ctx, dt);
    RunSpriteAnimationSystem(*this, ctx, dt);
    RunLightUpdateSystem(*this, ctx, dt);
    RunAnimationUpdateSystem(*this, ctx, dt);
    RunFacingUpdateSystem(*this, ctx, dt);
    ctx.Wait();
    // transform, update local matrix from position, rotation and scale
    RunTransformationUpdateSystem(*this, ctx, dt);
    ctx.Wait();
    // hierarchy, update world matrix based on hierarchy
    RunHierarchyUpdateSystem(*this, ctx, dt);
    ctx.Wait();

    RunSkeletonUpdateSystem(*this, ctx, dt);
    ctx.Wait();

    // update bounding box
    RunMeshAABBUpdateSystem(*this, ctx, dt);

    // @TODO: refactor
    for (auto [entity, camera, transform] : view<CameraComponent, TransformComponent>()) {
        if (camera.update(transform.worldMatrix())) {
            dirtyFlags_.fetch_or(SCENE_DIRTY_CAMERA);
        }
    }

    flushPendingDestroy();
}

void Scene::begin(Owner<SceneRuntime>&& runtime) {
    if (m_runtime) {
        LOG_ERROR(LogChannel::Scene, "onSimBegin already called");
        return;
    }

    m_runtime = std::move(runtime);
    m_runtime->start();

    update(0.0f);
}

void Scene::end() {
    if (m_runtime) {
        m_runtime->shutdown();
        m_runtime.reset();
    }
}

void Scene::tick(SceneTickContext ctx) {
    if (m_runtime) {
        m_runtime->update(ctx);
    }

    update(ctx.dt);
}

void Scene::copy(const Scene& other) {
    ComponentId idx = 0;
    for (auto& entry : other.m_storage.entries()) {
        if (entry.pool) {
            m_storage.ensure(idx);
            m_storage.m_entries[idx].pool = std::move(entry.pool->clone());
        }
        ++idx;
    }

    m_root = other.m_root;
    m_world_bound = other.m_world_bound;
    m_entity_seed = other.m_entity_seed;
}

Vector<Entity> Scene::getSortedEntityArray() const {
    HashSet<Entity> entity_set;

    for (const auto& it : m_storage.entries()) {
        if (!it.pool) continue;
        for (auto entity : it.pool->entityArray()) {
            if (has<PrefabChildComponent>(entity)) {
                continue;
            }
            entity_set.insert(entity);
        }
    }

    Vector<Entity> entity_array(entity_set.begin(), entity_set.end());

    std::sort(entity_array.begin(), entity_array.end());
    return entity_array;
}

void Scene::flushPendingDestroy() {
    const size_t pending_count = count<PendingDestroyComponent>();
    if (pending_count == 0) {
        return;
    }

    Vector<Entity> entities;
    entities.reserve(pending_count);
    for (auto [ent, _] : view<PendingDestroyComponent>()) {
        entities.emplace_back(ent);
    }

    for (auto ent : entities) {
        removeEntity(ent);
    }
}

bool Scene::has(ComponentId cid, Entity ent) const {
    return m_storage.has(cid, ent);
}

size_t Scene::count(ComponentId cid) const {
    if (const ecs::IComponentPool* pool = m_storage.tryGet(cid)) {
        return pool->count();
    }

    return 0;
}

bool Scene::remove(ComponentId cid, Entity ent) {
    return m_storage.remove(cid, ent);
}

Entity Scene::findFirstByName(std::string_view name) const {
    for (auto [entity, name_component] : view<NameComponent>()) {
        if (name_component.name() == name) {
            return entity;
        }
    }
    return Entity::null();
}

Entity Scene::findChildByName(std::string_view name, Entity ent) const {
    for (auto [entity, hier, name_component] : view<HierarchyComponent, NameComponent>()) {
        if (hier.parent_id == ent && name_component.name() == name) {
            return entity;
        }
    }

    return Entity::null();
}

void Scene::removeEntity(Entity ent) {
    // @TODO: move it to SceneCommandExecutor
    if (!ent.valid()) {
        return;
    }

    Vector<Entity> children;
    for (auto [child, hierarchy] : view<HierarchyComponent>()) {
        if (hierarchy.parent_id == ent) {
            children.emplace_back(child);
        }
    }

    for (auto child : children) {
        removeEntity(child);
    }

    for (auto& e : m_storage.entries()) {
        if (e.pool) {
            e.pool->remove(ent);
        }
    }
}

void Scene::remapEntity(const HashMap<Entity, Entity>& mapping) {
    // remap hierarchy
    for (auto [id, hier] : view<HierarchyComponent>()) {
        auto it = mapping.find(hier.parent_id);
        DEV_ASSERT(it != mapping.end());
        hier.parent_id = it->second;
    }

    // remap material
    for (auto [id, renderer] : view<MeshRendererComponent>()) {
        auto& materials = renderer.GetMaterialInstances();
        for (size_t i = 0; i < materials.size(); ++i) {
            const auto it = mapping.find(materials[i]);
            DEV_ASSERT(it != mapping.end());
            materials[i] = it->second;
        }

        CRASH_NOW_MSG("remap skin and skeleton");
    }
}

void Scene::attachChild(Entity child, Entity parent) {
    DEV_ASSERT(child != parent);
    DEV_ASSERT(parent.valid());

    // @TODO: prevent circular dependency

    HierarchyComponent* hier = component<HierarchyComponent>(child);

    if (hier == nullptr) {
        hier = &create<HierarchyComponent>(child);
    }

    hier->parent_id = parent;
}

template<typename T>
static void DuplicateComponent(Scene& scene, Entity source, Entity dest) {
    if (const T* comp = scene.component<T>(source)) {
        T copy = *comp;
        scene.create<T>(dest) = copy;
    }
}

Entity Scene::duplicateEntity(Entity ent) {
    if (!ent.valid()) {
        return ent;
    }

    Entity entity = createEntity();

#define REGISTER_COMPONENT(COMP, ...) DuplicateComponent<COMP>(*this, ent, entity);
    REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

    return entity;
}

}  // namespace cave
