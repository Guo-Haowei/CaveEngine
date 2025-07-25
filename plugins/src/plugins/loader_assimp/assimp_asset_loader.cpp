#include "assimp_asset_loader.h"

#if USING(USING_ASSIMP)
// #include <assimp/GltfMaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "engine/assets/mesh_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

namespace cave {

auto AssimpAssetLoader::Load() -> Result<AssetRef> {
    m_scene = new Scene;
    Assimp::Importer importer;

    const uint32_t flag = aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs;

    std::string resolved_path = FileAccess::FixPath(FileAccess::ACCESS_RESOURCE, m_import_path);
    const aiScene* aiscene = importer.ReadFile(resolved_path, flag);

    // check for errors
    if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode) {
        return CAVE_ERROR(ErrorCode::FAILURE, "Error: failed to import scene '{}'\n\tdetails: {}", m_filePath, importer.GetErrorString());
    }

    const uint32_t numMeshes = aiscene->mNumMeshes;
    const uint32_t numMaterials = aiscene->mNumMaterials;

    // LOG_VERBOSE("scene '{}' has {} meshes, {} materials", m_file_path, numMeshes, numMaterials);

    for (uint32_t i = 0; i < numMaterials; ++i) {
        ProcessMaterial(*aiscene->mMaterials[i]);
    }

    std::shared_ptr<MeshAsset> mesh_asset;
    for (uint32_t i = 0; i < numMeshes; ++i) {
        mesh_asset = ProcessMesh(*aiscene->mMeshes[i]);
    }

#if 0
    ecs::Entity root = ProcessNode(aiscene->mRootNode, ecs::Entity::Null());
    m_scene->GetComponent<NameComponent>(root)->SetName(m_meta.import_path);

    m_scene->m_root = root;
#endif
    return AssetRef(mesh_asset);
}

void AssimpAssetLoader::ProcessMaterial(aiMaterial& p_material) {
    unused(p_material);
#if 0
    MaterialComponent* materialComponent = m_scene->GetComponent<MaterialComponent>(material_id);
    DEV_ASSERT(materialComponent);

    auto get_material_path = [&](aiTextureType p_type, uint32_t p_index) -> std::string {
        aiString path;
        if (p_material.GetTexture(p_type, p_index, &path) == AI_SUCCESS) {
            return m_basePath + path.C_Str();
        }
        return "";
    };

    std::string path = get_material_path(aiTextureType_DIFFUSE, 0);
    if (!path.empty()) {
        materialComponent->textures[MaterialComponent::TEXTURE_BASE].path = path;
        DEV_ASSERT(0);
        // AssetRegistry::GetSingleton().RequestAssetSync(path);
    }

    path = get_material_path(aiTextureType_NORMALS, 0);
    if (path.empty()) {
        path = get_material_path(aiTextureType_HEIGHT, 0);
    }
    if (!path.empty()) {
        materialComponent->textures[MaterialComponent::TEXTURE_NORMAL].path = path;
        DEV_ASSERT(0);
        // AssetRegistry::GetSingleton().RequestAssetSync(path);
    }

    path = get_material_path(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE);
    if (!path.empty()) {
        materialComponent->textures[MaterialComponent::TEXTURE_METALLIC_ROUGHNESS].path = path;
        AssetRegistry::GetSingleton().RequestAssetSync(path);
    }

    m_materials.emplace_back(material_id);
#endif
}

std::shared_ptr<MeshAsset> AssimpAssetLoader::ProcessMesh(const aiMesh& p_mesh) {
    DEV_ASSERT(p_mesh.mNumVertices);
    const std::string mesh_name(p_mesh.mName.C_Str());
    const bool has_uv = p_mesh.mTextureCoords[0];
    if (!has_uv) {
        LOG_WARN("mesh {} does not have texture coordinates", mesh_name);
    }
    mesh_name;

    auto mesh_asset = std::make_shared<MeshAsset>();

    for (uint32_t i = 0; i < p_mesh.mNumVertices; ++i) {
        auto& position = p_mesh.mVertices[i];
        mesh_asset->positions.emplace_back(Vector3f(position.x, position.y, position.z));
        auto& normal = p_mesh.mNormals[i];
        mesh_asset->normals.emplace_back(Vector3f(normal.x, normal.y, normal.z));
        if (p_mesh.mTangents) {
            auto& tangent = p_mesh.mTangents[i];
            mesh_asset->tangents.emplace_back(Vector3f(tangent.x, tangent.y, tangent.z));
        }

        if (has_uv) {
            auto& uv = p_mesh.mTextureCoords[0][i];
            mesh_asset->texcoords_0.emplace_back(Vector2f(uv.x, uv.y));
        } else {
            mesh_asset->texcoords_1.emplace_back(Vector2f(0));
        }
    }

    for (uint32_t i = 0; i < p_mesh.mNumFaces; ++i) {
        aiFace& face = p_mesh.mFaces[i];
        mesh_asset->indices.emplace_back(face.mIndices[0]);
        mesh_asset->indices.emplace_back(face.mIndices[1]);
        mesh_asset->indices.emplace_back(face.mIndices[2]);
    }

    // DEV_ASSERT(m_materials.size());
    MeshAsset::MeshSubset subset;
    subset.index_count = (uint32_t)mesh_asset->indices.size();
    subset.index_offset = 0;
    // CRASH_NOW();
    // subset.material_id = m_materials.at(p_mesh.mMaterialIndex);
    mesh_asset->subsets.emplace_back(subset);

    mesh_asset->CreateRenderData();

    return mesh_asset;
}

ecs::Entity AssimpAssetLoader::ProcessNode(const aiNode* p_node, ecs::Entity p_parent) {
    const auto key = std::string(p_node->mName.C_Str());

    ecs::Entity entity;

    if (p_node->mNumMeshes == 1) {  // geometry node
        entity = EntityFactory::CreateObjectEntity(*m_scene, "Geometry::" + key);

        MeshRendererComponent& objComponent = *m_scene->GetComponent<MeshRendererComponent>(entity);
        objComponent.SetResourceGuid(Guid());
        //objComponent.meshId = m_meshes[p_node->mMeshes[0]];
    } else {  // else make it a transform/bone node
        entity = EntityFactory::CreateTransformEntity(*m_scene, "Node::" + key);

        for (uint32_t i = 0; i < p_node->mNumMeshes; ++i) {
            ecs::Entity child = EntityFactory::CreateObjectEntity(*m_scene, "");
            auto tagComponent = m_scene->GetComponent<NameComponent>(child);
            tagComponent->SetName("SubGeometry_" + std::to_string(child.GetId()));
            MeshRendererComponent& objComponent = *m_scene->GetComponent<MeshRendererComponent>(child);
            objComponent.SetResourceGuid(Guid());
            //= m_meshes[p_node->mMeshes[i]];
            m_scene->AttachChild(child, entity);
        }
    }

    DEV_ASSERT(m_scene->Contains<TransformComponent>(entity));

    const aiMatrix4x4& local = p_node->mTransformation;                           // row major matrix
    Matrix4x4f localTransformColumnMajor(local.a1, local.b1, local.c1, local.d1,  // x0 y0 z0 w0
                                         local.a2, local.b2, local.c2, local.d2,  // x1 y1 z1 w1
                                         local.a3, local.b3, local.c3, local.d3,  // x2 y2 z2 w2
                                         local.a4, local.b4, local.c4, local.d4   // x3 y3 z3 w3
    );
    TransformComponent& transform = *m_scene->GetComponent<TransformComponent>(entity);
    transform.MatrixTransform(localTransformColumnMajor);

    if (p_parent.IsValid()) {
        m_scene->AttachChild(entity, p_parent);
    }

    // process children
    for (uint32_t childIndex = 0; childIndex < p_node->mNumChildren; ++childIndex) {
        ProcessNode(p_node->mChildren[childIndex], entity);
    }

    return entity;
}

}  // namespace cave
#endif
