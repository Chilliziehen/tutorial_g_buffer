/*
 * filename：Vertex
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for Vertex.
 * Could be extended.
 */
#ifndef IMMEDIATERENDERERVK_VERTEX_H
#define IMMEDIATERENDERERVK_VERTEX_H

#include <glm.hpp>
namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for Vertex.
     * @note Could be extended.
     */
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
}
#endif //IMMEDIATERENDERERVK_VERTEX_H