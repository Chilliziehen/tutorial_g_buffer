/*
* filename：Light.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#pragma once

#ifndef INITIALGL_LIGHT_H
#define INITIALGL_LIGHT_H

#include <string>
#include "glm.hpp"
#include <GL/glew.h>

class Light {
public:
    glm::vec3 position{2.0f, 2.0f, 2.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float constant{1.0f};
    float linear{0.09f};
    float quadratic{0.032f};

    Light() = default;
    Light(const glm::vec3& pos, const glm::vec3& col,
          float kc = 1.0f, float kl = 0.09f, float kq = 0.032f)
        : position(pos), color(col), constant(kc), linear(kl), quadratic(kq) {}

    // 将本光源上传到 programId 的 lights[index] 上
    void upload(GLuint programId, int index, const std::string& uniformArrayName = "lights") const;
};

#endif
