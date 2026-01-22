//
// Created by 30367 on 2025/10/28.
//

#ifndef INITIALGL_UTILITIES_H
#define INITIALGL_UTILITIES_H
#include <glm.hpp>
#include <string>
#include <GL/glew.h>
#include <gtc/type_ptr.hpp>

#include "data_structures/RenderScene.h"

namespace Utilities {
    const char* rdFile(const char* filePath);

    // Uniform helpers: defined inline in header to avoid ODR/link issues across TUs.
    inline void setInt(uint32_t programId, const char* name, int v) {
        const GLint loc = glGetUniformLocation(static_cast<GLuint>(programId), name);
        if (loc != -1) glUniform1i(loc, v);
    }
    inline void setFloat(uint32_t programId, const char* name, float v) {
        const GLint loc = glGetUniformLocation(static_cast<GLuint>(programId), name);
        if (loc != -1) glUniform1f(loc, v);
    }
    inline void setVec3(uint32_t programId, const char* name, const glm::vec3& v) {
        const GLint loc = glGetUniformLocation(static_cast<GLuint>(programId), name);
        if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(v));
    }
    inline void setVec4(uint32_t programId, const char* name, const glm::vec4& v) {
        const GLint loc = glGetUniformLocation(static_cast<GLuint>(programId), name);
        if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(v));
    }
    inline void setMat4(uint32_t programId, const char* name, const glm::mat4& m) {
        const GLint loc = glGetUniformLocation(static_cast<GLuint>(programId), name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    }

    uint32_t createTexture2DFromFile(const std::string& path) ;
    void drainGlErrors(const char* tag);
    void calculateGlobalTransforms(scene::RenderScene* rs);
}

#endif //INITIALGL_UTILITIES_H

