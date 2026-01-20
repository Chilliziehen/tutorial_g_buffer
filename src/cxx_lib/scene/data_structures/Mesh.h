/*
 * filename：Mesh
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for Mesh.
 * Could be extended.
 * Attention: Actual vertex and index data are not stored here.
 */
#ifndef IMMEDIATERENDERERVK_MESH_H
#define IMMEDIATERENDERERVK_MESH_H
#include <string>
#include <glm.hpp>
namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for SubMesh.
     * @note Could be extended.
     * @attention Actual vertex and index data are not stored here.
     */
    struct SubMesh {
        uint32_t baseVertex;    ///@note base offset in VERTEX BUFFER.(Or Vertex data in HOST MEMORY)
        uint32_t baseIndex;     ///@note base offset in INDEX BUFFER.(Or Index data in HOST MEMORY)
        uint32_t indexCount;    ///@note number of indices.
        int materialIndex;      ///@note index of material in corresponding material list.
        glm::vec3 minAABB;      ///@note minimum AABB. Used for frustum culling.
        glm::vec3 maxAABB;      ///@note maximum AABB. Used for frustum culling.
    };
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for Mesh.
     * @note Could be extended.
     * @attention Actual vertex and index data are not stored here.
     */
    struct Mesh {
        std::string name;
        std::vector<SubMesh> subMeshes;
    };
}

#endif //IMMEDIATERENDERERVK_MESH_H