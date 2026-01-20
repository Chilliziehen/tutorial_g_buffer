/*
* filename：Light.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "Light.h"
#include <string>

static inline void setVec3(GLuint programId, const std::string& name, const glm::vec3& v) {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform3f(loc, v.x, v.y, v.z);
}

static inline void setFloat(GLuint programId, const std::string& name, float v) {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1f(loc, v);
}

void Light::upload(GLuint programId, int index, const std::string& uniformArrayName) const {
    const std::string base = uniformArrayName + "[" + std::to_string(index) + "]";
    setVec3(programId, base + ".position", position);
    setVec3(programId, base + ".color", color);
    setFloat(programId, base + ".constant", constant);
    setFloat(programId, base + ".linear", linear);
    setFloat(programId, base + ".quadratic", quadratic);
}

