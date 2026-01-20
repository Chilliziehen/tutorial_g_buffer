/*
* filename：LightManager.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "LightManager.h"

static inline void setInt(GLuint programId, const std::string& name, int v) {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1i(loc, v);
}

void LightManager::upload(GLuint programId, const std::string& uniformArrayName, const std::string& numUniformName) const {
    const int n = static_cast<int>(std::min<size_t>(lights.size(), MAX_LIGHTS));
    setInt(programId, numUniformName, n);
    for (int i = 0; i < n; ++i) {
        lights[i].upload(programId, i, uniformArrayName);
    }
}
// filepath: d:\Repositories\ComputerGraphics\InitialGL\src\cxx\main\light\Light.h


