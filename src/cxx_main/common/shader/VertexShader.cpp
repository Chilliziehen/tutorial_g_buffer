/*
* filename：VertexShader.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "VertexShader.h"
#include "../util/Utilities.h"
#include <stdexcept>

VertexShader::VertexShader(const char *vertexPath) {
    try {
        this->vshSrc = Utilities::rdFile(vertexPath);
        this->compileSuccess = false;
        this->vshID = -1;
    }
    catch (std::runtime_error& e) {
        throw std::runtime_error("[Vertex Shader][Constructor] "+std::string(e.what()));
    }
}

VertexShader::~VertexShader() {
    glDeleteShader(this->vshID);
    if (this->isCompiled()) {
        glDeleteShader(this->vshID);
    }
}

void VertexShader::compile() {
    try {
        this->vshID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(this->vshID, 1, &this->vshSrc, nullptr);
        glCompileShader(this->vshID);
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(this->vshID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(this->vshID, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("vertex shader compilation failed: ") + std::string(infoLog));
        }
        this->compileSuccess = true;
    }
    catch (std::runtime_error& e) {
        throw std::runtime_error("[Vertex Shader][Compile] "+std::string(e.what()));
    }
}

GLuint VertexShader::getId() {
    if (!this->compileSuccess) {
        throw std::runtime_error("[Vertex Shader][Get ID] Vertex shader has not been compiled yet.");
    }
    return this->vshID;
}

bool VertexShader::isCompiled() {
    return this->compileSuccess;
}