/*
 * filename：SceneRenderer_GL
 */

#include "SceneRenderer_GL.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>

#include <algorithm>

namespace {
static inline void setInt(GLuint programId, const char* name, int v) {
    const GLint loc = glGetUniformLocation(programId, name);
    if (loc != -1) glUniform1i(loc, v);
}

static inline void setMat4(GLuint programId, const char* name, const glm::mat4& m) {
    const GLint loc = glGetUniformLocation(programId, name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

static inline glm::mat4 toMat4(const scene::Transform& t) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, t.position);
    M *= glm::mat4_cast(t.rotation);
    M = glm::scale(M, t.scale);
    return M;
}

static inline int mapTexIndex(const std::unordered_map<int, int>& mapping, int sceneTexIndex) {
    if (sceneTexIndex < 0) return -1;
    const auto it = mapping.find(sceneTexIndex);
    if (it == mapping.end()) return -1;
    return it->second;
}
} // namespace

namespace scene {

void drawSceneNode_GL(const RenderScene& rs,
                   int nodeIndex,
                   const glm::mat4& parent,
                   GLuint programId,
                   const std::unordered_map<int, int>& textureIndexBySceneTexture) {
    if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= rs.nodes.size()) return;

    const Node& node = rs.nodes[nodeIndex];
    const glm::mat4 local = toMat4(node.localTransform);
    const glm::mat4 model = parent * local;

    setMat4(programId, "uModel", model);

    if (node.meshIndex >= 0 && static_cast<size_t>(node.meshIndex) < rs.meshes.size()) {
        const Mesh& mesh = rs.meshes[node.meshIndex];
        for (const SubMesh& sm : mesh.subMeshes) {
            const int matIndex = sm.materialIndex;

            // Default: no material/texture.
            int txAlbedo = -1;
            int txNormal = -1;
            int txAO = -1;
            int txMR = -1;
            int txEmissive = -1;

            if (matIndex >= 0 && static_cast<size_t>(matIndex) < rs.materials.size()) {
                const Material& m = rs.materials[matIndex];
                txAlbedo = mapTexIndex(textureIndexBySceneTexture, m.albedoMapIndex);
                txNormal = mapTexIndex(textureIndexBySceneTexture, m.normalMapIndex);
                txAO = mapTexIndex(textureIndexBySceneTexture, m.aoMapIndex);
                txMR = mapTexIndex(textureIndexBySceneTexture, m.metallicRoughnessMapIndex);
                txEmissive = mapTexIndex(textureIndexBySceneTexture, m.emissiveMapIndex);
            }

            setInt(programId, "uMaterialIndex", matIndex);
            setInt(programId, "uTexIndexAlbedo", txAlbedo);
            setInt(programId, "uTexIndexNormal", txNormal);
            setInt(programId, "uTexIndexAO", txAO);
            setInt(programId, "uTexIndexMetallicRoughness", txMR);
            setInt(programId, "uTexIndexEmissive", txEmissive);

            glDrawElementsBaseVertex(GL_TRIANGLES,
                                    static_cast<GLsizei>(sm.indexCount),
                                    GL_UNSIGNED_INT,
                                    reinterpret_cast<void*>(static_cast<uintptr_t>(sm.baseIndex) * sizeof(uint32_t)),
                                    static_cast<GLint>(sm.baseVertex));
        }
    }

    for (const int child : node.children) {
        drawSceneNode_GL(rs, child, model, programId, textureIndexBySceneTexture);
    }
}

} // namespace scene

