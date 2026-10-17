#include "PathTracer.h"

#include "BvhAccel.h"

#include "cave/core/time/Stopwatch.h"
#include "cave/render/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/MaterialComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/render/rhi/RenderDevice.h"
#include "engine/private/runtime/assets/MeshAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {
#include "shader_resource_defines.slang.h"
}  // namespace cave

namespace cave::render {

namespace {

struct GpuScene {
    Vector<GpuPtBvh> bvhs;
    Vector<GpuPtVertex> vertices;
    Vector<GpuPtIndex> indices;
};

template<typename T>
auto CreateBuffer(IRenderDevice* device, const std::vector<T>& data) {
    auto desc = GpuResourceDesc::structuredBuffer(sizeof(T), static_cast<uint32_t>(data.size()));
    desc.initial_data = data.data();

    return device->createStructuredBuffer(desc);
}

void ConstructMesh(const MeshAsset& mesh, GpuScene& gpu_scene) {
    if (!mesh.bvh) {
        mesh.bvh = BvhAccel::construct(mesh.indices, mesh.positions);
    }

    mesh.bvh->fillGpuBvhAccel(gpu_scene.bvhs);
    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        GpuPtVertex vertex;
        vertex.position = mesh.positions[i];
        vertex.normal = mesh.normals[i];
        gpu_scene.vertices.emplace_back(vertex);
    }

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        GpuPtIndex index;
        index.tri = math::Vec3i(mesh.indices[i],
                                mesh.indices[i + 1],
                                mesh.indices[i + 2]);
        gpu_scene.indices.emplace_back(index);
    }
}

void AppendVertices(std::span<const GpuPtVertex> vertices,
                    std::vector<GpuPtVertex>& out) {
#if USING(ENABLE_ASSERT)
    const size_t offset = out.size();
    const size_t count = vertices.size();
#endif

    out.insert(out.end(), vertices.begin(), vertices.end());

    DEV_ASSERT(out.size() == offset + count);
}

void AppendIndices(std::span<const GpuPtIndex> indices,
                   std::vector<GpuPtIndex>& out,
                   int vertex_count) {
    const int offset = (int)out.size();
    const int count = (int)indices.size();
    out.resize(offset + count);
    for (int i = 0; i < count; ++i) {
        const auto& source = indices[i];
        auto& dest = out[i + offset];
        dest.tri = source.tri + vertex_count;
    }
}

void AppendBvhs(std::span<const GpuPtBvh> bvhs,
                std::vector<GpuPtBvh>& out,
                int index_offset) {
    const auto offset = static_cast<int>(out.size());
    auto adjust_index = [offset](int& index) {
        if (index >= 0) {
            index += offset;
        }
    };

    const int count = (int)bvhs.size();
    out.resize(offset + count);

    for (int i = 0; i < count; ++i) {
        const auto& source = bvhs[i];
        auto& dest = out[i + offset];
        dest = source;

        adjust_index(dest.hit_idx);
        adjust_index(dest.miss_idx);
        if (dest.triangleIndex >= 0) {
            dest.triangleIndex += index_offset;
        }
    }
}

}  // namespace

void PathTracer::update(const Scene& scene) {
    switch (m_mode) {
        case PathTracerMode::NONE:
            return;
        case PathTracerMode::TILED:
            LOG_ERROR("Not implmeneted");
            [[fallthrough]];
        case PathTracerMode::INTERACTIVE:
            break;
        default:
            CRASH_NOW_MSG("Invalid mode");
            return;
    }

    if (!m_index_buffer) {
        createAccelStructure(scene);
    }

    std::vector<GpuPtMesh> meshes;
    updateAccelStructure(scene);
    // @TODO: update mesh buffers
}

void PathTracer::updateAccelStructure(const Scene& scene) {
    const auto dirty_flag = SCENE_DIRTY_NONE;
    // @TODO: refactor
    auto device = RenderDevice::singletonPtr();

    HashMap<ecs::Entity, int> materials_lut;

    if ((dirty_flag != SCENE_DIRTY_NONE) || m_material_buffer == nullptr) {
        auto view = scene.view<MaterialComponent>();
        Vector<GpuPtMaterial> materials;

        GpuPtMaterial default_mat{
            .base_color = math::Vec3f::One,
            .roughness = 0.9f,
            .emissive = math::Vec3f::Zero,
            .metallic = 0.1f,
        };
        materials.push_back(default_mat);
        materials_lut[ecs::Entity::null()] = 0;

        for (auto [id, material] : view) {
            const int idx = static_cast<int>(materials.size());
            auto [it, ok] = materials_lut.try_emplace(id, idx);
            if (!ok) continue;

            GpuPtMaterial gpu_mat;
            gpu_mat.base_color = material.base_color.xyz;
            gpu_mat.emissive = gpu_mat.base_color * material.emissive;
            gpu_mat.roughness = material.roughness;
            gpu_mat.metallic = material.metallic;
            materials.emplace_back(gpu_mat);
        }

        auto desc = GpuResourceDesc::structuredBuffer(sizeof(materials[0]), static_cast<uint32_t>(materials.size()));
        desc.initial_data = materials.data();

        m_material_buffer = *device->createStructuredBuffer(desc);
    }

    if ((dirty_flag & SCENE_DIRTY_WORLD) || m_mesh_buffer == nullptr) {
        auto view = scene.view<MeshRendererComponent, TransformComponent>();

        Vector<GpuPtMesh> meshes;
        meshes.reserve(scene.count<MeshRendererComponent>());
        for (auto [id, renderer, transform] : view) {
            if (!renderer.visible()) continue;

            auto handle = renderer.meshHandle();
            auto mesh = handle.get();
            if (DEV_VERIFY(mesh)) {
                auto mesh_it = m_meshs.find(handle.guid());
                if (mesh_it == m_meshs.end()) {
                    CRASH_NOW_MSG("mesh not found");
                    continue;
                }

                const auto& materials = renderer.materialInstances();
                const ecs::Entity material_id = materials.empty() ? ecs::Entity::null() : materials[0];

                const auto& cache = mesh_it->second;
                GpuPtMesh gpu_pt_mesh;
                gpu_pt_mesh.transform = transform.worldMatrix();
                gpu_pt_mesh.transform_inv = glm::inverse(gpu_pt_mesh.transform);
                gpu_pt_mesh.root_bvh_id = cache.root_bvh_id;
                auto mat_it = materials_lut.find(material_id);
                DEV_ASSERT(mat_it != materials_lut.end());
                gpu_pt_mesh.material_idx = mat_it->second;

                meshes.push_back(gpu_pt_mesh);
            }
        }

        auto desc = GpuResourceDesc::structuredBuffer(sizeof(meshes[0]), static_cast<uint32_t>(meshes.size()));
        desc.initial_data = meshes.data();

        m_mesh_buffer = *device->createStructuredBuffer(desc);
    }
}

bool PathTracer::createAccelStructure(const Scene& scene) {
    DEV_ASSERT(m_vertex_buffer == nullptr);

    // @TODO: refactor
    auto device = RenderDevice::singletonPtr();
    GpuScene gpu_scene;

    Stopwatch stopwatch;
    stopwatch.start();

    // meshes
    for (auto [id, renderer] : scene.view<MeshRendererComponent>()) {
        auto transform = scene.component<TransformComponent>(id);
        auto handle = renderer.meshHandle();
        auto mesh = handle.get();
        if (DEV_VERIFY(transform && mesh)) {
            auto it = m_meshs.find(handle.guid());
            if (it != m_meshs.end()) {
                continue;
            }

            const int bvh_count = (int)gpu_scene.bvhs.size();
            const int index_count = (int)gpu_scene.indices.size();
            const int vertex_count = (int)gpu_scene.vertices.size();

            MeshData meta{
                .root_bvh_id = bvh_count,
            };
            m_meshs[handle.guid()] = meta;

            GpuScene tmp_scene;
            ConstructMesh(*mesh, tmp_scene);

            AppendVertices(tmp_scene.vertices, gpu_scene.vertices);
            AppendIndices(tmp_scene.indices, gpu_scene.indices, vertex_count);
            AppendBvhs(tmp_scene.bvhs, gpu_scene.bvhs, index_count);
        }
    }

    const uint32_t triangle_count = (uint32_t)gpu_scene.indices.size();
    const uint32_t bvh_count = (uint32_t)gpu_scene.bvhs.size();

    m_bvh_buffer = *CreateBuffer(device, gpu_scene.bvhs);
    m_vertex_buffer = *CreateBuffer(device, gpu_scene.vertices);
    m_index_buffer = *CreateBuffer(device, gpu_scene.indices);

    stopwatch.stop();
    LOG_INFO("Path tracer scene loaded in {}, contains {} triangles, {} BVH",
             stopwatch.elapsed().ToString(),
             triangle_count,
             bvh_count);

    /// materials
#if 0
    for (auto [entity, material] : scene.view<MaterialComponent>()) {
        auto fill_texture = [&](int p_index, int& p_out_enabled, sampler2D& p_out_handle) {
            const ImageAsset* image = AssetRegistry::singleton().GetAssetByHandle<ImageAsset>(material.textures[p_index].path);
            if (image && image->gpu_texture) {
                uint64_t handle = image->gpu_texture->GetResidentHandle();
                if (handle) {
                    p_out_enabled = true;
                    if (is_opengl) {
                        p_out_handle.Set64(handle);

                    } else {
                        p_out_handle.Set32(static_cast<int>(handle));
                    }
                    return true;
                }
            }
            return false;
        };

        fill_texture(MaterialComponent::TEXTURE_BASE, gpu_mat.has_base_color_map, gpu_mat.base_color_map_handle);
        fill_texture(MaterialComponent::TEXTURE_NORMAL, gpu_mat.has_normal_map, gpu_mat.normal_map_handle);
        fill_texture(MaterialComponent::TEXTURE_METALLIC_ROUGHNESS, gpu_mat.has_material_map, gpu_mat.material_map_handle);

        p_out_scene.materials.push_back(gpu_mat);
        material_lut[entity] = static_cast<int>(p_out_scene.materials.size()) - 1;
    }

    /// objects
    p_out_scene.geometries.clear();
    for (auto [entity, light] : p_scene.m_LightComponents) {
        if (light.type() == LIGHT_TYPE_INFINITE) {
            auto transform = p_scene.GetComponent<TransformComponent>(entity);
            DEV_ASSERT(transform);
            Vec3f rotation = (transform->GetWorldMatrix() * Vecf::UnitZ).xyz;
            rotation = normalize(rotation);
            float radius = 1000.0f;

            Vec3f tmp;
            tmp.Set(&rotation.x);
            Vec3f center = radius * 0.5f * tmp;

            auto it = material_lut.find(entity);
            DEV_ASSERT(it != material_lut.end());
            int material_id = it->second;
            gpu_geometry_t sphere(center, radius, material_id);
            // HACK: increase emissive power
            p_out_scene.materials[material_id].emissive *= 5.0f;
            // gpu_geometry_t sphere(center, radius, it->second);
            tmp_gpu_objects.push_back(sphere);
        }
    }
#endif

    return true;
}

bool PathTracer::isActive() const {
    if (m_mode != PathTracerMode::INTERACTIVE) {
        return false;
    }

    if (m_bvh_buffer == nullptr) {
        return false;
    }

    return true;
}

}  // namespace cave::render
