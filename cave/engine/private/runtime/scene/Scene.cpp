#include "Scene.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/threading/JobSystem.h"
#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneRuntime.h"
#include "cave/runtime/tile_map/TileSetAsset.h"
#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/core/io/archive.h"
#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/framework/Engine.h"
#include "engine/private/systems/AnimationSystem.h"
#include "engine/private/systems/EcsSystems.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

Scene::Scene(const MetaRegistry& reg) noexcept
    : m_component_registry(reg) {
}

Scene::Scene() noexcept
    : Scene(engine::GetMetaRegistry()) {
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
        LOG_ERROR(LogChannel::Scene, "runtime already created");
        return;
    }

    m_runtime = std::move(runtime);
    m_runtime->start(false);

    update(0.0f);

    DEV_ASSERT(m_hierarchy.validate(*this));
}

void Scene::alwaysRun(Owner<SceneRuntime>&& runtime) {
    if (m_runtime) {
        LOG_ERROR(LogChannel::Scene, "runtime already created");
        return;
    }

    m_runtime = std::move(runtime);
    m_runtime->start(true);

    update(0.0f);

    m_runtime->shutdown();
    m_runtime.reset();
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
    for (auto& entry : other.m_storage.entries()) {
        if (entry.pool) {
            const uint32_t idx = m_storage.ensure(entry.type_id);
            m_storage.m_entries[idx].pool = std::move(entry.pool->clone());
        }
    }

    m_world_bound = other.m_world_bound;
    m_entity_seed = other.m_entity_seed;

    m_hierarchy.rebuild(*this);
}

Vector<Entity> Scene::getSortedEntityArray() const {
    HashSet<Entity> entity_set;

    for (const auto& it : m_storage.entries()) {
        if (!it.pool) continue;
        for (auto entity : it.pool->entityArray()) {
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
        removeEntityImpl(ent);
    }

    m_hierarchy.rebuild(*this);
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

bool Scene::removeComponent(ComponentId cid, Entity ent) {
    if (cid == HierarchyComponent_Id) {
        CRASH_NOW_MSG("shouldn't delete Hiearachy from this");
    }

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
        if (hier.parent() == ent && name_component.name() == name) {
            return entity;
        }
    }

    return Entity::null();
}

Entity Scene::activeCamera() const {
    for (auto [entity, camera, transform] : view<CameraComponent, TransformComponent>()) {
        if (entity.valid())
            return entity;
    }

    return Entity::null();
}

void Scene::removeEntity(Entity ent) {
    if (ent.valid()) {
        removeEntityImpl(ent);
        m_hierarchy.rebuild(*this);
    }
}

void Scene::removeEntityImpl(Entity ent) {
    // @TODO: move it to SceneCommandExecutor
    if (!ent.valid()) {
        return;
    }

    Vector<Entity> children;
    for (auto [child, hierarchy] : view<HierarchyComponent>()) {
        if (hierarchy.parent() == ent) {
            children.emplace_back(child);
        }
    }

    for (auto child : children) {
        removeEntityImpl(child);
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
        auto it = mapping.find(hier.parent());
        DEV_ASSERT(it != mapping.end());

        hier.setParentRaw(it->second);
    }

    // remap material
    for (auto [id, renderer] : view<MeshRendererComponent>()) {
        auto& materials = renderer.materialInstances();
        for (size_t i = 0; i < materials.size(); ++i) {
            const auto it = mapping.find(materials[i]);
            DEV_ASSERT(it != mapping.end());
            materials[i] = it->second;
        }

        const auto it = mapping.find(renderer.skeletonId());
        DEV_ASSERT(it != mapping.end());
        renderer.setSkeletonId(it->second);
    }

    for (uint16_t cid = 0; cid < static_cast<uint16_t>(storage().entries().size()); ++cid) {
        auto& pool = storage().entries()[cid].pool;
        if (pool) {
            pool->remap(mapping);
        }
    }
}

bool Scene::attachChild(Entity child, Entity parent) {
    if (!child.valid()) {
        return false;
    }

    if (parent.valid()) {
        if (isChild(parent, child)) {
            return false;
        }
    }

    HierarchyComponent* hier = component<HierarchyComponent>(child);
    DEV_ASSERT(hier);

    const Entity old_parent = hier ? hier->parent() : Entity::null();
    if (old_parent == parent) {
        return false;
    }

    hier->setParentRaw(parent);
    m_hierarchy.onParentChanged(child, old_parent, parent);
    return true;
}

bool Scene::isChild(Entity child, Entity parent) const {
    if (!DEV_VERIFY(child.valid() && parent.valid())) {
        return false;
    }

    if (child == parent) {
        return false;
    }

    Entity cursor = child;
    while (cursor.valid()) {
        const auto* hier = component<HierarchyComponent>(cursor);
        if (DEV_VERIFY(hier)) {
            if (hier->parent() == parent) return true;
            cursor = hier->parent();
        }
    }
    return false;
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

namespace {

const TileFrame* FindTileFrame(const TileDefinition& definition,
                               float elapsed_time) {
    if (definition.animation.empty()) {
        return nullptr;
    }

    float total_duration = 0.0f;
    for (const TileFrame& frame : definition.animation) {
        total_duration += std::max(frame.duration, 0.0f);
    }

    if (total_duration <= 0.0f) {
        return &definition.animation.front();
    }

    float local_time = std::fmod(elapsed_time, total_duration);
    if (local_time < 0.0f) {
        local_time += total_duration;
    }

    for (const TileFrame& frame : definition.animation) {
        const float duration = std::max(frame.duration, 0.0f);

        if (local_time < duration) {
            return &frame;
        }

        local_time -= duration;
    }

    // Handles floating-point precision near total_duration.
    return &definition.animation.back();
}

// @TODO: move to animation system
void SubmitTileLayer(float dt, ICanvas& canvas,
                     const TileMapInstanceComponent::LayerCache& layer,
                     const TransformComponent& transform) {
    const ImageAsset* image = layer.image.get();
    const TileSetAsset* tile_set = layer.tile_set.get();
    if (!image || !tile_set) {
        return;
    }

    const auto& frames = tile_set->frames();
    for (const auto& tile : layer.tiles) {
        tile.elapsed += dt;
        const auto* definition = tile_set->getTileDefinition(tile.tile_id);
        if (!definition) continue;

        uint32_t atlas_index = definition->id;
        if (!definition->animation.empty()) {
            auto* frame = FindTileFrame(*definition, tile.elapsed);
            if (!frame) continue;
            atlas_index = frame->atlas_index;
        }

        if (atlas_index >= frames.size()) continue;
        const auto frame = frames[atlas_index];

        ImageDrawOptions options;
        options.z_index = layer.z_index;
        options.transform = &transform.worldMatrix();
        options.uv_min = frame.min();
        options.uv_max = frame.max();

        const float s = 1.0f;
        float x0 = s * tile.x;
        float y0 = s * tile.y;
        float x1 = s * (tile.x + 1);
        float y1 = s * (tile.y + 1);

        canvas.addImage(image->gpu_texture.get(),
                        Vec2f(x0, y0),
                        Vec2f(x1, y1),
                        options);
    }
}

void SubmitTileMap(float dt, Scene& scene, ICanvas& canvas) {
    auto view = scene.view<TileMapInstanceComponent, TransformComponent, HierarchyComponent>();

    for (const auto& [id, instance, transform, hier] : view) {
        if (!hier.visible()) continue;

        instance.createRenderData();

        instance.tileMapHandle();

        auto layers = instance.layers();
        if (layers.empty()) continue;

        for (const auto& layer : layers) {
            SubmitTileLayer(dt, canvas, layer, transform);
        }
    }
}

}  // namespace

void Scene::submit2D(float dt, ICanvas& canvas) {
    SubmitTileMap(dt, *this, canvas);
}

}  // namespace cave
