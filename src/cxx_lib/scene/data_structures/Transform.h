/*
 * filename：Transform
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for Transform.
 * Could be extended.
 */
#ifndef IMMEDIATERENDERERVK_TRANSFORM_H
#define IMMEDIATERENDERERVK_TRANSFORM_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @attention Member funtion getMatrix() return LOCAL MODEL Matrix.
     * @brief Data structure for Transform.
     * @note Could be extended.
     */
    struct Transform {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        /**
         * @brief Get the transformation matrix combining position, rotation, and scale.
         * @return glm::mat4 The resulting transformation matrix.
         * @attention Return value is LOCAL MODEL Matrix.
         */
        [[nodiscard]] glm::mat4 getMatrix() const {
            return glm::translate(glm::mat4(1.0f), position) *
                   glm::toMat4(rotation) *
                   glm::scale(glm::mat4(1.0f), scale);
        }
    };
}
#endif //IMMEDIATERENDERERVK_TRANSFORM_H