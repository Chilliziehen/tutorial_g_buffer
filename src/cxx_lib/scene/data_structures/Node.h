/*
 * filename：Node
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for glTF scene node.
 * Could be extended.
 * Node is double-directed to its parent and children.
 */
#ifndef IMMEDIATERENDERERVK_NODE_H
#define IMMEDIATERENDERERVK_NODE_H
#include <optional>
#include <string>
#include <vector>
#include "Transform.h"
#include <glm.hpp>
namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for glTF scene node.
     * @note Could be extended.
     * @attention related indices stored in its source RenderScene object.
     * @ref RenderScene.h
     */
    struct Node {
        std::string name;
        int parentIndex = -1;
        std::vector<int> children;
        Transform localTransform;
        std::optional<glm::mat4> globalTransform;
        int meshIndex = -1;
        int cameraIndex = -1;
        int lightIndex = -1;
    };
}

#endif //IMMEDIATERENDERERVK_NODE_H