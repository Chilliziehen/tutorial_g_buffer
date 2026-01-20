//
// Created by 30367 on 2025/10/28.
//

#ifndef INITIALGL_UTILITIES_H
#define INITIALGL_UTILITIES_H
#include <glm.hpp>
#include <GL/glew.h>

namespace Utilities {
    const char* rdFile(const char* filePath);
    static inline void setInt(GLuint programId, const char* name, int v);
    static inline void setFloat(GLuint programId, const char* name, float v);
    static inline void setVec3(GLuint programId, const char* name, const glm::vec3& v);
    static inline void setVec4(GLuint programId, const char* name, const glm::vec4& v);
    static inline void setMat4(GLuint programId, const char* name, const glm::mat4& m);
    static GLuint createTexture2DFromFile(const std::string& path) ;
    static void drainGlErrors(const char* tag);
}

#endif //INITIALGL_UTILITIES_H