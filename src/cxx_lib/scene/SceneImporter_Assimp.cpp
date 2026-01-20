/*
 * filename：SceneImporter_Assimp
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Implementation for SceneImporter using Assimp library.
 */

#include "SceneImporter_Assimp.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm.hpp>
#include <gtc/quaternion.hpp>

namespace scene {
    SceneImporter_Assimp::~SceneImporter_Assimp() {
        this->importedScene.reset();
    }

    bool SceneImporter_Assimp::loadScene(const std::string &filePath) {
        Assimp::Importer importer;
        constexpr unsigned int postProcessFlags =
                aiProcess_Triangulate |
                aiProcess_JoinIdenticalVertices |
                aiProcess_GenSmoothNormals |
                aiProcess_CalcTangentSpace |
                aiProcess_ValidateDataStructure;

        const aiScene* assimpScene = importer.ReadFile(filePath, postProcessFlags);
        if (assimpScene == nullptr || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 || assimpScene->mRootNode == nullptr) {
            this->importedScene.reset();
            return false;
        }

        RenderScene out;

        const std::filesystem::path srcPath(filePath);
        const std::filesystem::path baseDir = srcPath.has_parent_path() ? srcPath.parent_path() : std::filesystem::path{};

        auto normalizeTexturePath = [&](const std::string& raw) -> std::string {
            if (raw.empty()) {
                return raw;
            }
            if (!raw.empty() && raw[0] == '*') {
                // Assimp embedded texture reference like "*0".
                return raw;
            }
            std::filesystem::path p(raw);
            if (p.is_relative() && !baseDir.empty()) {
                p = baseDir / p;
            }
            return p.lexically_normal().string();
        };

        std::unordered_map<std::string, int> textureKeyToIndex;
        auto getOrCreateTexture = [&](const aiMaterial* mat, aiTextureType type, const std::string& logicalType) -> int {
            if (mat == nullptr) {
                return -1;
            }
            if (mat->GetTextureCount(type) <= 0) {
                return -1;
            }
            aiString texPath;
            if (mat->GetTexture(type, 0, &texPath) != AI_SUCCESS) {
                return -1;
            }
            std::string normalizedPath = normalizeTexturePath(texPath.C_Str());
            if (normalizedPath.empty()) {
                return -1;
            }
            const std::string key = normalizedPath + "|" + logicalType;
            const auto it = textureKeyToIndex.find(key);
            if (it != textureKeyToIndex.end()) {
                return it->second;
            }
            TextureInfo info;
            info.path = normalizedPath;
            info.type = logicalType;
            out.textures.push_back(info);
            const int index = static_cast<int>(out.textures.size()) - 1;
            textureKeyToIndex.emplace(key, index);
            return index;
        };

        // Materials
        out.materials.reserve(assimpScene->mNumMaterials);
        for (unsigned int i = 0; i < assimpScene->mNumMaterials; ++i) {
            const aiMaterial* mat = assimpScene->mMaterials[i];
            Material m;

            aiString name;
            if (mat != nullptr && mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
                m.name = name.C_Str();
            }

            aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f);
            if (mat != nullptr) {
                // Prefer glTF PBR base color if present, else diffuse.
                if (mat->Get(AI_MATKEY_BASE_COLOR, baseColor) != AI_SUCCESS) {
                    mat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
                }
            }
            m.baseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);

            float metallic = 1.0f;
            float roughness = 1.0f;
            if (mat != nullptr) {
                mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
                mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
            }
            m.metallicFactor = metallic;
            m.roughnessFactor = roughness;

            int twoSided = 0;
            if (mat != nullptr) {
                mat->Get(AI_MATKEY_TWOSIDED, twoSided);
            }
            m.doubleSided = (twoSided != 0);

            // Best-effort alpha mode (glTF specific property may not always exist)
            if (mat != nullptr) {
                aiString alphaMode;
#ifdef AI_MATKEY_GLTF_ALPHAMODE
                if (mat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
                    m.alphaMode = alphaMode.C_Str();
                }
#endif
            }

            // Textures
            // Note: Assimp splits/aliases glTF texture types depending on importer.
            m.albedoMapIndex = getOrCreateTexture(mat, aiTextureType_BASE_COLOR, "albedo");
            if (m.albedoMapIndex < 0) {
                m.albedoMapIndex = getOrCreateTexture(mat, aiTextureType_DIFFUSE, "albedo");
            }
            m.normalMapIndex = getOrCreateTexture(mat, aiTextureType_NORMALS, "normal");

            // Metallic-Roughness: prefer METALNESS, else DIFFUSE_ROUGHNESS if importer provides it.
            m.metallicRoughnessMapIndex = getOrCreateTexture(mat, aiTextureType_METALNESS, "metallicRoughness");
            if (m.metallicRoughnessMapIndex < 0) {
                m.metallicRoughnessMapIndex = getOrCreateTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "metallicRoughness");
            }

            m.emissiveMapIndex = getOrCreateTexture(mat, aiTextureType_EMISSIVE, "emissive");
            m.aoMapIndex = getOrCreateTexture(mat, aiTextureType_AMBIENT_OCCLUSION, "ao");

            out.materials.push_back(m);
        }

        // Primitive submeshes and global buffers
        std::vector<SubMesh> primitiveSubMeshes;
        primitiveSubMeshes.resize(assimpScene->mNumMeshes);

        out.globalVertices.reserve(assimpScene->mNumMeshes * 1024);
        out.globalIndices.reserve(assimpScene->mNumMeshes * 1024);

        for (unsigned int meshIndex = 0; meshIndex < assimpScene->mNumMeshes; ++meshIndex) {
            const aiMesh* mesh = assimpScene->mMeshes[meshIndex];
            if (mesh == nullptr) {
                continue;
            }

            SubMesh sub{};
            sub.baseVertex = static_cast<uint32_t>(out.globalVertices.size());
            sub.baseIndex = static_cast<uint32_t>(out.globalIndices.size());
            sub.materialIndex = static_cast<int>(mesh->mMaterialIndex);

            glm::vec3 minAABB(
                    std::numeric_limits<float>::infinity(),
                    std::numeric_limits<float>::infinity(),
                    std::numeric_limits<float>::infinity());
            glm::vec3 maxAABB(
                    -std::numeric_limits<float>::infinity(),
                    -std::numeric_limits<float>::infinity(),
                    -std::numeric_limits<float>::infinity());

            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                Vertex outV{};

                const aiVector3D pos = mesh->mVertices[v];
                outV.position = glm::vec3(pos.x, pos.y, pos.z);

                if (mesh->HasNormals()) {
                    const aiVector3D n = mesh->mNormals[v];
                    outV.normal = glm::vec3(n.x, n.y, n.z);
                }
                if (mesh->HasTextureCoords(0)) {
                    const aiVector3D uv = mesh->mTextureCoords[0][v];
                    outV.texCoord = glm::vec2(uv.x, uv.y);
                }
                if (mesh->HasTangentsAndBitangents()) {
                    const aiVector3D t = mesh->mTangents[v];
                    const aiVector3D b = mesh->mBitangents[v];
                    outV.tangent = glm::vec3(t.x, t.y, t.z);
                    outV.bitangent = glm::vec3(b.x, b.y, b.z);
                }

                minAABB = glm::min(minAABB, outV.position);
                maxAABB = glm::max(maxAABB, outV.position);

                out.globalVertices.push_back(outV);
            }

            uint32_t indexCount = 0;
            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                const aiFace& face = mesh->mFaces[f];
                for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                    out.globalIndices.push_back(sub.baseVertex + static_cast<uint32_t>(face.mIndices[j]));
                    ++indexCount;
                }
            }

            sub.indexCount = indexCount;
            sub.minAABB = minAABB;
            sub.maxAABB = maxAABB;

            primitiveSubMeshes[meshIndex] = sub;
        }

        // Node hierarchy
        out.nodes.reserve(256);

        auto convertTransform = [](const aiMatrix4x4& m) -> Transform {
            Transform t;
            aiVector3D scaling;
            aiQuaternion rotation;
            aiVector3D position;
            m.Decompose(scaling, rotation, position);
            t.position = glm::vec3(position.x, position.y, position.z);
            t.scale = glm::vec3(scaling.x, scaling.y, scaling.z);
            t.rotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
            return t;
        };

        std::function<int(const aiNode*, int)> addNode = [&](const aiNode* srcNode, int parentIndex) -> int {
            Node n;
            n.name = (srcNode != nullptr && srcNode->mName.length > 0) ? srcNode->mName.C_Str() : std::string{};
            n.parentIndex = parentIndex;
            if (srcNode != nullptr) {
                n.localTransform = convertTransform(srcNode->mTransformation);
            }

            // Mesh binding: group node primitives into one Mesh entry.
            if (srcNode != nullptr && srcNode->mNumMeshes > 0) {
                Mesh nodeMesh;
                nodeMesh.name = n.name.empty() ? "Mesh" : (n.name + "_Mesh");
                nodeMesh.subMeshes.reserve(srcNode->mNumMeshes);
                for (unsigned int k = 0; k < srcNode->mNumMeshes; ++k) {
                    const unsigned int primIndex = srcNode->mMeshes[k];
                    if (primIndex < primitiveSubMeshes.size()) {
                        nodeMesh.subMeshes.push_back(primitiveSubMeshes[primIndex]);
                    }
                }
                out.meshes.push_back(nodeMesh);
                n.meshIndex = static_cast<int>(out.meshes.size()) - 1;
            }

            const int thisIndex = static_cast<int>(out.nodes.size());
            out.nodes.push_back(n);

            if (srcNode != nullptr) {
                Node& thisNode = out.nodes[thisIndex];
                thisNode.children.reserve(srcNode->mNumChildren);
                for (unsigned int c = 0; c < srcNode->mNumChildren; ++c) {
                    const aiNode* child = srcNode->mChildren[c];
                    const int childIndex = addNode(child, thisIndex);
                    thisNode.children.push_back(childIndex);
                }
            }

            return thisIndex;
        };

        // Coordinate system conversion: Y-up -> Z-up.
        // We apply this as an extra transform on the root so vertex data stays in source space.
        const glm::quat yUp_to_zUp = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        auto applyRotationToTransform = [&](Transform& t) {
            t.position = yUp_to_zUp * t.position;
            t.rotation = yUp_to_zUp * t.rotation;
        };

        const aiNode* srcRoot = assimpScene->mRootNode;
        const int importedRootIndex = addNode(srcRoot, -1);

        // If the source file effectively has multiple top-level nodes (no single unified root),
        // create a virtual root node and attach all top-level nodes under it.
        // Heuristic: root has multiple children and no meshes.
        const bool needVirtualRoot = (srcRoot != nullptr && srcRoot->mNumChildren > 1 && srcRoot->mNumMeshes == 0);
        if (needVirtualRoot) {
            Node virtualRoot;
            virtualRoot.name = "__VirtualRoot__";
            virtualRoot.parentIndex = -1;
            virtualRoot.localTransform = Transform{};
            virtualRoot.localTransform.rotation = yUp_to_zUp;

            const int virtualRootIndex = static_cast<int>(out.nodes.size());
            out.nodes.push_back(virtualRoot);

            // Re-parent imported root's children directly under the virtual root.
            // Note: importedRootIndex is a synthetic node created from Assimp root.
            Node& importedRoot = out.nodes[importedRootIndex];
            Node& newRoot = out.nodes[virtualRootIndex];

            newRoot.children = importedRoot.children;
            for (const int childIndex : newRoot.children) {
                if (childIndex >= 0 && static_cast<size_t>(childIndex) < out.nodes.size()) {
                    out.nodes[childIndex].parentIndex = virtualRootIndex;
                }
            }

            // Keep imported root node in the pool but detach it from being the root.
            importedRoot.children.clear();

            out.rootIndex = virtualRootIndex;
        } else {
            // Single unified root: apply coordinate conversion on that root.
            applyRotationToTransform(out.nodes[importedRootIndex].localTransform);
            out.rootIndex = importedRootIndex;
        }

        this->importedScene = std::move(out);
        return true;
    }

    RenderScene& SceneImporter_Assimp::getImportedScene() {
        if (!this->importedScene.has_value()) {
            throw std::runtime_error("SceneImporter_Assimp::getImportedScene() called before a successful loadScene().");
        }
        return this->importedScene.value();
    }

} // scene