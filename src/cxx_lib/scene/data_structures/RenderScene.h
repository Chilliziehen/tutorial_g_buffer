/*
 * filename：RenderScene
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for RenderScene.
 * Could be extended.
 * THIS STRUCTURE IMPLEMENTS RESOURCE POOLING PATTERN.
 */
#ifndef IMMEDIATERENDERERVK_RENDERSCENE_H
#define IMMEDIATERENDERERVK_RENDERSCENE_H
#include <vector>
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "TextureInfo.h"
#include "Vertex.h"

namespace scene {
    /**
     * @brief Data structure for RenderScene.
     * @author Chilliziehen scream303677483@gmail.com
     * @note Could be extended.
     * @attention THIS STRUCTURE IMPLEMENTS RESOURCE POOLING PATTERN.
     */
    struct RenderScene {
        std::vector<TextureInfo> textures;
        std::vector<Material> materials;
        std::vector<Mesh> meshes;
        std::vector<Node> nodes;
        int rootIndex = -1; /// @attention Should be unique. -1 means no root node defined and the scene is bad.

        //Global Resource Pools.
        std::vector<Vertex> globalVertices;
        std::vector<uint32_t> globalIndices;
    };
}

#endif //IMMEDIATERENDERERVK_RENDERSCENE_H