#include "tiny_gltf_importer.h"

#if USING(USE_IMPORTER_TINYGLTF)

#include "engine/assets/material_asset.h"
#include "engine/assets/mesh_asset.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION

WARNING_PUSH()
WARNING_DISABLE(4018, "-Wconversion")
WARNING_DISABLE(4267, "-Wconversion")
#include "tinygltf/tiny_gltf.h"
WARNING_POP()

namespace tinygltf {

static bool DummyLoadImage(Image*,
                           const int,
                           std::string*,
                           std::string*,
                           int,
                           int,
                           const unsigned char*,
                           int,
                           void*) {
    return true;
}

static bool DummyWriteImage(const std::string* /* basepath */,
                            const std::string* /* filename */,
                            const Image* /* image */,
                            bool /* embedImages */,
                            const FsCallbacks* /* fs_cb */,
                            const URICallbacks* /* uri_cb */,
                            std::string* /* out_uri */,
                            void* /* user_pointer */) {
    return true;
}

}  // namespace tinygltf

namespace cave {

namespace fs = std::filesystem;

Result<void> TinyGltfImporter::Import() {
    if (auto res = PrepareImport(); !res) {
        return CAVE_ERROR(res.error());
    }

    m_scene = std::make_shared<Scene>();

    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    m_model = std::make_shared<tinygltf::Model>();

    std::string source_path = m_source_path.string();
    loader.SetImageLoader(tinygltf::DummyLoadImage, nullptr);
    loader.SetImageWriter(tinygltf::DummyWriteImage, nullptr);
    bool ret = loader.LoadASCIIFromFile(m_model.get(), &err, &warn, source_path);

    if (!warn.empty()) {
        return CAVE_ERROR(ErrorCode::FAILURE, "Warn: failed to import scene '{}'\n\tdetails: {}", source_path, warn);
    }

    if (!err.empty()) {
        return CAVE_ERROR(ErrorCode::FAILURE, "Error: failed to import scene '{}'\n\tdetails: {}", source_path, err);
    }

    if (!ret) {
        return CAVE_ERROR(ErrorCode::FAILURE, "Error: failed to import scene '{}'", source_path);
    }

    ecs::Entity root = m_scene->CreateEntity();
    m_scene->Create<TransformComponent>(root);
    m_scene->Create<NameComponent>(root);
    m_scene->m_root = root;

    for (const tinygltf::Material& mat : m_model->materials) {
        m_materials.emplace_back(ProcessMaterial(mat));
    }

    for (const tinygltf::Mesh& mesh : m_model->meshes) {
        m_meshes.emplace_back(ProcessMesh(mesh));
    }

    // Create skeleton
    for (const auto& skin : m_model->skins) {
        ecs::Entity skeleton_id = m_scene->CreateEntity();
        m_scene->Create<NameComponent>(skeleton_id).SetName(skin.name);
        m_scene->Create<TransformComponent>(skeleton_id);
        SkeletonComponent& skeleton = m_scene->Create<SkeletonComponent>(skeleton_id);
        if (skin.inverseBindMatrices >= 0) {
            const tinygltf::Accessor& accessor = m_model->accessors[skin.inverseBindMatrices];
            const tinygltf::BufferView& buffer_view = m_model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = m_model->buffers[buffer_view.buffer];
            skeleton.inverse_bind_matrices.resize(accessor.count);
            memcpy(skeleton.inverse_bind_matrices.data(), &buffer.data[accessor.byteOffset + buffer_view.byteOffset], accessor.count * sizeof(Matrix4x4f));
        } else {
            LOG_FATAL("No inverse matrices found");
        }
    }

    // Create transform hierarchy, assign objects, meshes, skeletons, cameras
    DEV_ASSERT(m_model->scenes.size());
    const tinygltf::Scene& gltf_scene = m_model->scenes[max(0, m_model->defaultScene)];
    for (size_t i = 0; i < gltf_scene.nodes.size(); ++i) {
        ProcessNode(gltf_scene.nodes[i], root);
    }

    // Create skeleton-bone mappings:
    for (size_t skin_index = 0; skin_index < m_model->skins.size(); ++skin_index) {
        const tinygltf::Skin& skin = m_model->skins[skin_index];
        SkeletonComponent& skeleton = m_scene->GetComponentByIndex<SkeletonComponent>(skin_index);

        const size_t joint_count = skin.joints.size();
        skeleton.bone_collection.resize(joint_count);

        // create bone collection
        for (size_t i = 0; i < joint_count; ++i) {
            int jointIndex = skin.joints[i];
            ecs::Entity bone_id = m_node_map[jointIndex];
            skeleton.bone_collection[i] = bone_id;
        }
    }

    // Create animations:
    for (const tinygltf::Animation& anim : m_model->animations) {
        ProcessAnimation(anim);
    }

    // @TODO: lights
    // @TODO: cameras

    return RegisterScene(root);
}

Guid TinyGltfImporter::ProcessMaterial(const tinygltf::Material& p_material) {
    const auto& x = p_material;
    std::string name = x.name.empty() ? GenerateMaterialName() : x.name;

    auto material_asset = std::make_shared<MaterialAsset>();

    auto base_color = x.values.find("baseColorFactor");
    auto roughness = x.values.find("roughnessFactor");
    auto metallic = x.values.find("metallicFactor");

    auto base_color_tex = x.values.find("baseColorTexture");
    auto normal_tex = x.additionalValues.find("normalTexture");
    auto metallic_roughness_tex = x.values.find("metallicRoughnessTexture");
    // auto emissiveTexture = x.additionalValues.find("emissiveTexture");
    // auto occlusionTexture = x.additionalValues.find("occlusionTexture");
    // auto emissiveFactor = x.additionalValues.find("emissiveFactor");
    // auto alphaCutoff = x.additionalValues.find("alphaCutoff");
    // auto alphaMode = x.additionalValues.find("alphaMode");

    if (base_color != x.values.end()) {
        const auto& number_array = base_color->second.number_array;
        for (size_t idx = 0; idx < number_array.size(); ++idx) {
            material_asset->base_color[idx] = static_cast<float>(number_array[idx]);
        }
    }

    if (roughness != x.values.end()) {
        material_asset->roughness = static_cast<float>(roughness->second.number_value);
    }

    if (metallic != x.values.end()) {
        material_asset->metallic = static_cast<float>(metallic->second.number_value);
    }

    if (base_color_tex != x.values.end()) {
        auto& tex = m_model->textures[base_color_tex->second.TextureIndex()];
        int img_source = tex.source;
        auto& img = m_model->images[img_source];
        const std::string path = m_base_path + img.uri;
        Guid guid = RegisterImage(path, true).value();
        material_asset->textures[std::to_underlying(TextureSlot::Base)] = guid;
    }

    if (normal_tex != x.additionalValues.end()) {
        auto& tex = m_model->textures[normal_tex->second.TextureIndex()];
        int img_source = tex.source;
        auto& img = m_model->images[img_source];
        const std::string path = m_base_path + img.uri;
        Guid guid = RegisterImage(path, false).value();
        material_asset->textures[std::to_underlying(TextureSlot::Normal)] = guid;
    }

    if (metallic_roughness_tex != x.values.end()) {
        auto& tex = m_model->textures[metallic_roughness_tex->second.TextureIndex()];
        int img_source = tex.source;
        auto& img = m_model->images[img_source];
        const std::string path = m_base_path + img.uri;
        Guid guid = RegisterImage(path, false).value();
        material_asset->textures[std::to_underlying(TextureSlot::MetallicRoughness)] = guid;
    }

    return RegisterMaterial(std::move(name), std::move(material_asset)).value();
}

Guid TinyGltfImporter::ProcessMesh(const tinygltf::Mesh& p_mesh) {
    std::string name = p_mesh.name.empty() ? GenerateMeshName() : p_mesh.name;

    auto mesh_asset = std::make_shared<MeshAsset>();
    MeshAsset& mesh = *mesh_asset;

    for (const auto& prim : p_mesh.primitives) {
        MeshAsset::MeshSubset subset;

        const uint32_t vertexOffset = (uint32_t)mesh.normals.size();

        if (prim.indices >= 0) {
            // Fill indices:
            const tinygltf::Accessor& accessor = m_model->accessors[prim.indices];
            const tinygltf::BufferView& buffer_view = m_model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = m_model->buffers[buffer_view.buffer];

            int stride = accessor.ByteStride(buffer_view);
            size_t index_count = accessor.count;
            size_t index_offset = mesh.indices.size();
            mesh.indices.resize(index_offset + index_count);
            subset.index_offset = (uint32_t)index_offset;
            subset.index_count = (uint32_t)index_count;
            mesh.subsets.emplace_back(subset);

            const uint8_t* data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

            if (stride == 1) {
                for (size_t index = 0; index < index_count; index += 3) {
                    mesh.indices[index_offset + index + 0] = vertexOffset + data[index + 0];
                    mesh.indices[index_offset + index + 1] = vertexOffset + data[index + 1];
                    mesh.indices[index_offset + index + 2] = vertexOffset + data[index + 2];
                }
            } else if (stride == 2) {
                for (size_t index = 0; index < index_count; index += 3) {
                    mesh.indices[index_offset + index + 0] = vertexOffset + ((uint16_t*)data)[index + 0];
                    mesh.indices[index_offset + index + 1] = vertexOffset + ((uint16_t*)data)[index + 1];
                    mesh.indices[index_offset + index + 2] = vertexOffset + ((uint16_t*)data)[index + 2];
                }
            } else if (stride == 4) {
                for (size_t index = 0; index < index_count; index += 3) {
                    mesh.indices[index_offset + index + 0] = vertexOffset + ((uint32_t*)data)[index + 0];
                    mesh.indices[index_offset + index + 1] = vertexOffset + ((uint32_t*)data)[index + 1];
                    mesh.indices[index_offset + index + 2] = vertexOffset + ((uint32_t*)data)[index + 2];
                }
            } else {
                CRASH_NOW_MSG("unsupported index stride!");
            }
        }

        for (auto& attr : prim.attributes) {
            const std::string& attrName = attr.first;
            int attrData = attr.second;

            const tinygltf::Accessor& accessor = m_model->accessors[attrData];
            const tinygltf::BufferView& buffer_view = m_model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = m_model->buffers[buffer_view.buffer];

            int stride = accessor.ByteStride(buffer_view);
            size_t vertexCount = accessor.count;

            if (mesh.subsets.back().index_count == 0) {
                CRASH_NOW_MSG("This is not common");
            }

            const uint8_t* data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

            if (attrName == "POSITION") {
                mesh.positions.resize(vertexOffset + vertexCount);
                for (size_t index = 0; index < vertexCount; ++index) {
                    mesh.positions[vertexOffset + index] = *(const Vector3f*)(data + index * stride);
                }

                if (accessor.sparse.isSparse) {
                    auto& sparse = accessor.sparse;
                    const tinygltf::BufferView& sparse_indices_view = m_model->bufferViews[sparse.indices.bufferView];
                    const tinygltf::BufferView& sparse_values_view = m_model->bufferViews[sparse.values.bufferView];
                    const tinygltf::Buffer& sparse_indices_buffer = m_model->buffers[sparse_indices_view.buffer];
                    const tinygltf::Buffer& sparse_values_buffer = m_model->buffers[sparse_values_view.buffer];
                    const uint8_t* sparse_indices_data = sparse_indices_buffer.data.data() + sparse.indices.byteOffset + sparse_indices_view.byteOffset;
                    const uint8_t* sparse_values_data = sparse_values_buffer.data.data() + sparse.values.byteOffset + sparse_values_view.byteOffset;
                    switch (sparse.indices.componentType) {
                        default:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.positions[sparse_indices_data[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.positions[((const uint16_t*)sparse_indices_data)[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.positions[((const uint32_t*)sparse_indices_data)[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                    }
                }
            } else if (attrName == "NORMAL") {
                mesh.normals.resize(vertexOffset + vertexCount);
                for (size_t index = 0; index < vertexCount; ++index) {
                    mesh.normals[vertexOffset + index] = *(const Vector3f*)(data + index * stride);
                }

                if (accessor.sparse.isSparse) {
                    auto& sparse = accessor.sparse;
                    const tinygltf::BufferView& sparse_indices_view = m_model->bufferViews[sparse.indices.bufferView];
                    const tinygltf::BufferView& sparse_values_view = m_model->bufferViews[sparse.values.bufferView];
                    const tinygltf::Buffer& sparse_indices_buffer = m_model->buffers[sparse_indices_view.buffer];
                    const tinygltf::Buffer& sparse_values_buffer = m_model->buffers[sparse_values_view.buffer];
                    const uint8_t* sparse_indices_data = sparse_indices_buffer.data.data() + sparse.indices.byteOffset + sparse_indices_view.byteOffset;
                    const uint8_t* sparse_values_data = sparse_values_buffer.data.data() + sparse.values.byteOffset + sparse_values_view.byteOffset;
                    switch (sparse.indices.componentType) {
                        default:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.normals[sparse_indices_data[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.normals[((const uint16_t*)sparse_indices_data)[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            for (int s = 0; s < sparse.count; ++s) {
                                mesh.normals[((const uint32_t*)sparse_indices_data)[s]] = ((const Vector3f*)sparse_values_data)[s];
                            }
                            break;
                    }
                }
            } else if (attrName == "TANGENT") {
                mesh.tangents.resize(vertexOffset + vertexCount);
                for (size_t index = 0; index < vertexCount; ++index) {
                    mesh.tangents[vertexOffset + index] = *(const Vector3f*)(data + index * stride);
                }
            } else if (attrName == "TEXCOORD_0") {
                mesh.texcoords_0.resize(vertexOffset + vertexCount);
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        const Vector2f& tex = *(const Vector2f*)((size_t)data + index * stride);

                        mesh.texcoords_0[vertexOffset + index].x = tex.x;
                        mesh.texcoords_0[vertexOffset + index].y = tex.y;
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        const uint8_t& s = *(uint8_t*)((size_t)data + index * stride + 0);
                        const uint8_t& t = *(uint8_t*)((size_t)data + index * stride + 1);

                        mesh.texcoords_0[vertexOffset + index].x = s / 255.0f;
                        mesh.texcoords_0[vertexOffset + index].y = t / 255.0f;
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        const uint16_t& s = *(uint16_t*)((size_t)data + index * stride + 0 * sizeof(uint16_t));
                        const uint16_t& t = *(uint16_t*)((size_t)data + index * stride + 1 * sizeof(uint16_t));

                        mesh.texcoords_0[vertexOffset + index].x = s / 65535.0f;
                        mesh.texcoords_0[vertexOffset + index].y = t / 65535.0f;
                    }
                }
            } else if (attrName == "TEXCOORD_1") {
            } else if (attrName == "TEXCOORD_2") {
            } else if (attrName == "TEXCOORD_3") {
            } else if (attrName == "TEXCOORD_4") {
            } else if (attrName == "JOINTS_0") {
                mesh.joints_0.resize(vertexOffset + vertexCount);
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    struct JointTmp {
                        uint8_t ind[4];
                    };

                    for (size_t index = 0; index < vertexCount; ++index) {
                        const JointTmp& joint = *(const JointTmp*)(data + index * stride);

                        mesh.joints_0[vertexOffset + index].x = joint.ind[0];
                        mesh.joints_0[vertexOffset + index].y = joint.ind[1];
                        mesh.joints_0[vertexOffset + index].z = joint.ind[2];
                        mesh.joints_0[vertexOffset + index].w = joint.ind[3];
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    struct JointTmp {
                        uint16_t ind[4];
                    };

                    for (size_t index = 0; index < vertexCount; ++index) {
                        const JointTmp& joint = *(const JointTmp*)(data + index * stride);

                        mesh.joints_0[vertexOffset + index].x = joint.ind[0];
                        mesh.joints_0[vertexOffset + index].y = joint.ind[1];
                        mesh.joints_0[vertexOffset + index].z = joint.ind[2];
                        mesh.joints_0[vertexOffset + index].w = joint.ind[3];
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    struct JointTmp {
                        uint32_t ind[4];
                    };

                    for (size_t index = 0; index < vertexCount; ++index) {
                        const JointTmp& joint = *(const JointTmp*)(data + index * stride);

                        mesh.joints_0[vertexOffset + index].x = joint.ind[0];
                        mesh.joints_0[vertexOffset + index].y = joint.ind[1];
                        mesh.joints_0[vertexOffset + index].z = joint.ind[2];
                        mesh.joints_0[vertexOffset + index].w = joint.ind[3];
                    }
                } else {
                    DEV_ASSERT(0);
                }
            } else if (attrName == "WEIGHTS_0") {
                mesh.weights_0.resize(vertexOffset + vertexCount);
                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        mesh.weights_0[vertexOffset + index] = *(Vector4f*)((size_t)data + index * stride);
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        const uint8_t& x = *(uint8_t*)((size_t)data + index * stride + 0);
                        const uint8_t& y = *(uint8_t*)((size_t)data + index * stride + 1);
                        const uint8_t& z = *(uint8_t*)((size_t)data + index * stride + 2);
                        const uint8_t& w = *(uint8_t*)((size_t)data + index * stride + 3);

                        mesh.weights_0[vertexOffset + index].x = x / 255.0f;
                        mesh.weights_0[vertexOffset + index].x = y / 255.0f;
                        mesh.weights_0[vertexOffset + index].x = z / 255.0f;
                        mesh.weights_0[vertexOffset + index].x = w / 255.0f;
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    for (size_t index = 0; index < vertexCount; ++index) {
                        const uint16_t& x = *(uint8_t*)((size_t)data + index * stride + 0 * sizeof(uint16_t));
                        const uint16_t& y = *(uint8_t*)((size_t)data + index * stride + 1 * sizeof(uint16_t));
                        const uint16_t& z = *(uint8_t*)((size_t)data + index * stride + 2 * sizeof(uint16_t));
                        const uint16_t& w = *(uint8_t*)((size_t)data + index * stride + 3 * sizeof(uint16_t));

                        mesh.weights_0[vertexOffset + index].x = x / 65535.0f;
                        mesh.weights_0[vertexOffset + index].x = y / 65535.0f;
                        mesh.weights_0[vertexOffset + index].x = z / 65535.0f;
                        mesh.weights_0[vertexOffset + index].x = w / 65535.0f;
                    }
                }
            } else if (attrName == "COLOR_0") {
                LOG_WARN("TODO: COLOR_0");
            } else {
                LOG_ERROR("Unknown attrib %s", attrName.c_str());
            }
        }

        // TODO: morph target
    }

    // iterate through weights
    if (mesh.normals.empty()) {
        CRASH_NOW_MSG("No normal detected");
    }

    mesh.CreateRenderData();

    return RegisterMesh(std::move(name), std::move(mesh_asset)).value();
}

void TinyGltfImporter::ProcessNode(int p_node_index, ecs::Entity p_parent) {
    if (p_node_index < 0 || m_node_map.count(p_node_index)) {
        return;
    }

    ecs::Entity node_id;
    auto& node = m_model->nodes[p_node_index];

#if 0
#endif

    if (node.mesh >= 0) {
        ecs::Entity mesh_instance = EntityFactory::CreateMeshInstance(*m_scene, "Node::" + node.name);
        MeshRendererComponent& renderer = *m_scene->GetComponent<MeshRendererComponent>(mesh_instance);
        renderer.SetResourceGuid(m_meshes.at(node.mesh));

        const tinygltf::Mesh& mesh = m_model->meshes[node.mesh];
        for (const auto& prim : mesh.primitives) {
            ecs::Entity material_id = m_scene->CreateEntity();
            renderer.AddMaterial(material_id);

            Guid material_guid = m_materials[prim.material];
            MaterialComponent& material_instance = m_scene->Create<MaterialComponent>(material_id);
            material_instance.SetResourceGuid(material_guid);
        }

        const bool has_skin = node.skin >= 0;
        if (!has_skin) {
            node_id = mesh_instance;
        } else {
            node_id = m_scene->GetEntityByIndex<SkeletonComponent>(node.skin);
            renderer.SetSkeletonId(node_id);
            m_scene->AttachChild(mesh_instance, node_id);
        }

    } else if (node.camera >= 0) {
        LOG_WARN("@TODO: camera");
    }

    // light

    // transform
    if (!node_id.IsValid()) {
        node_id = m_scene->CreateEntity();
        m_scene->Create<TransformComponent>(node_id);
        m_scene->Create<NameComponent>(node_id).SetName("Transform::" + node.name);
    }

    auto [_, ok] = m_node_map.try_emplace(p_node_index, node_id);
    DEV_ASSERT(ok);

    TransformComponent& transform = *m_scene->GetComponent<TransformComponent>(node_id);
    if (!node.matrix.empty()) {
        Matrix4x4f matrix;
        matrix[0].x = float(node.matrix.at(0));
        matrix[0].y = float(node.matrix.at(1));
        matrix[0].z = float(node.matrix.at(2));
        matrix[0].w = float(node.matrix.at(3));
        matrix[1].x = float(node.matrix.at(4));
        matrix[1].y = float(node.matrix.at(5));
        matrix[1].z = float(node.matrix.at(6));
        matrix[1].w = float(node.matrix.at(7));
        matrix[2].x = float(node.matrix.at(8));
        matrix[2].y = float(node.matrix.at(9));
        matrix[2].z = float(node.matrix.at(10));
        matrix[2].w = float(node.matrix.at(11));
        matrix[3].x = float(node.matrix.at(12));
        matrix[3].y = float(node.matrix.at(13));
        matrix[3].z = float(node.matrix.at(14));
        matrix[3].w = float(node.matrix.at(15));
        transform.MatrixTransform(matrix);
    } else {
        if (!node.scale.empty()) {
            // Note: limiting min scale because scale <= 0.0001 will break matrix decompose and mess up the model (float precision issue?)
            for (int idx = 0; idx < 3; ++idx) {
                if (std::abs(node.scale[idx]) <= 0.0001) {
                    const double sign = node.scale[idx] < 0 ? -1 : 1;
                    node.scale[idx] = 0.0001001 * sign;
                }
            }
            transform.SetScale(Vector3f(float(node.scale[0]), float(node.scale[1]), float(node.scale[2])));
        }
        if (!node.rotation.empty()) {
            transform.SetRotation(Vector4f(float(node.rotation[0]), float(node.rotation[1]), float(node.rotation[2]), float(node.rotation[3])));
        }
        if (!node.translation.empty()) {
            transform.SetTranslation(Vector3f(float(node.translation[0]), float(node.translation[1]), float(node.translation[2])));
        }
    }
    transform.UpdateTransform();

    if (p_parent.IsValid()) {
        m_scene->AttachChild(node_id, p_parent);
    }

    for (int child : node.children) {
        ProcessNode(child, node_id);
    }
}

void TinyGltfImporter::ProcessAnimation(const tinygltf::Animation& p_anim) {
    std::string tag = p_anim.name.empty() ? GenerateAnimationName() : p_anim.name;
    auto entity = EntityFactory::CreateNameEntity(*m_scene, tag);

    // @TODO: make animation asset instead
    m_scene->AttachChild(entity);

    SkeletalAnimationComponent& animation = m_scene->Create<SkeletalAnimationComponent>(entity);
    animation.samplers.resize(p_anim.samplers.size());
    animation.channels.resize(p_anim.channels.size());
    DEV_ASSERT(p_anim.samplers.size() == p_anim.channels.size());

    for (size_t index = 0; index < p_anim.samplers.size(); ++index) {
        const auto& gltf_sampler = p_anim.samplers[index];
        DEV_ASSERT(gltf_sampler.interpolation == "LINEAR");
        auto& sampler = animation.samplers[index];

        // Animation Sampler input = keyframe times
        {
            const tinygltf::Accessor& accessor = m_model->accessors[gltf_sampler.input];
            const tinygltf::BufferView& buffer_view = m_model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = m_model->buffers[buffer_view.buffer];

            DEV_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

            int stride = accessor.ByteStride(buffer_view);
            size_t count = accessor.count;

            sampler.keyframeTimes.resize(count);

            const unsigned char* data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

            DEV_ASSERT(stride == 4);

            for (size_t j = 0; j < count; ++j) {
                float time = ((float*)data)[j];
                sampler.keyframeTimes[j] = time;
                animation.start = min(animation.start, time);
                animation.end = max(animation.end, time);
            }
        }

        // Animation Sampler output = keyframe data
        {
            const tinygltf::Accessor& accessor = m_model->accessors[gltf_sampler.output];
            const tinygltf::BufferView& buffer_view = m_model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = m_model->buffers[buffer_view.buffer];

            int stride = accessor.ByteStride(buffer_view);
            size_t count = accessor.count;

            const unsigned char* data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

            switch (accessor.type) {
                case TINYGLTF_TYPE_SCALAR:
                    DEV_ASSERT(stride == sizeof(float));
                    break;
                case TINYGLTF_TYPE_VEC3:
                    DEV_ASSERT(stride == sizeof(Vector3f));
                    break;
                case TINYGLTF_TYPE_VEC4:
                    DEV_ASSERT(stride == sizeof(Vector4f));
                    break;
                default:
                    LOG_FATAL("Invalid format {}", accessor.type);
                    break;
            }
            sampler.keyframeData.resize(count * stride / sizeof(float));
            memcpy(sampler.keyframeData.data(), data, count * stride);
        }
    }

    for (size_t index = 0; index < p_anim.channels.size(); ++index) {
        const auto& channel = p_anim.channels[index];
        animation.channels[index].targetId = m_node_map[channel.target_node];
        DEV_ASSERT(channel.sampler >= 0);
        animation.channels[index].samplerIndex = (uint32_t)channel.sampler;

        if (channel.target_path == "scale") {
            animation.channels[index].path = AnimationChannelPath::Scale;
        } else if (channel.target_path == "rotation") {
            animation.channels[index].path = AnimationChannelPath::Rotation;
        } else if (channel.target_path == "translation") {
            animation.channels[index].path = AnimationChannelPath::Translation;
        } else {
            LOG_WARN("Unkown target path {}", channel.target_path.c_str());
            animation.channels[index].path = AnimationChannelPath::Unknown;
        }
    }
}

}  // namespace cave

#endif  // #if USING(USE_IMPORTER_TINYGLTF)
