/*
 * filename：SceneRenderer_GL
 * description：Utility draw for RenderScene on OpenGL.
 */
#pragma once

#include <unordered_map>
#include <GL/glew.h>
#include <glm.hpp>

#include "data_structures/RenderScene.h"

namespace scene {

// Draw a RenderScene node hierarchy. The caller must have bound the VAO/VBO/EBO that match
// RenderScene::globalVertices/globalIndices layout.
//
// Contract:
// - rs: imported scene
// - nodeIndex: node index in rs.nodes
// - parent: parent-to-world matrix passed from recursion
// - programId: active shader program (geometry pass)
// - textureIndexBySceneTexture: mapping from rs.textures[i] to a slot in the lighting pass texture array (0..127), or -1.
//
// This function sets uniforms:
// - uModel (mat4)
// - uMaterialIndex (int)
// - uTexIndexAlbedo/uTexIndexNormal/uTexIndexAO/uTexIndexMetallicRoughness/uTexIndexEmissive (int)
void drawSceneNode_GL(const RenderScene& rs,
                   int nodeIndex,
                   const glm::mat4& parent,
                   GLuint programId,
                   const std::unordered_map<int, int>& textureIndexBySceneTexture);

} // namespace scene

