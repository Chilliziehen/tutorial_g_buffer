/*
* filename：FragmentShader.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "../util/Utilities.h"
#include <stdexcept>
#include "FragmentShader.h"
#include <GL/glew.h>
FragmentShader::FragmentShader(const char* vertexPath) {
    try {
        this->fshSrc = Utilities::rdFile(vertexPath);
        this->compileSuccess = false;
        this->fshID = -1;
    }
    catch (std::runtime_error& e) {
        throw std::runtime_error("[Vertex Shader][Constructor] "+std::string(e.what()));
    }
}

FragmentShader::~FragmentShader() {
    glDeleteShader(this->fshID);
    if (this->fshSrc != nullptr) {
        delete[] this->fshSrc;
    }
    if (this->isCompiled()) {
        glDeleteShader(this->fshID);
    }
}

void FragmentShader::compile() {
    try {
        this->fshID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(this->fshID, 1, &this->fshSrc, nullptr);
        glCompileShader(this->fshID);
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(this->fshID, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(this->fshID, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("fragment shader compilation failed: ") + std::string(infoLog));
        }
        this->compileSuccess = true;
    }
    catch (std::runtime_error& e) {
        throw std::runtime_error("[Fragment Shader][Compile] "+std::string(e.what()));
    }
}

GLuint FragmentShader::getId() {
    if (!this->compileSuccess) {
        throw std::runtime_error("[Fragment Shader][Get ID] Vertex shader has not been compiled yet.");
    }
    return this->fshID;
}

bool FragmentShader::isCompiled() {
    return this->compileSuccess;
}