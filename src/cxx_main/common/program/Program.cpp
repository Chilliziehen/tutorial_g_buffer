/*
* filename：Program.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "Program.h"
#include <stdexcept>
#include "../shader/Shader.h"
Program::Program() {
    this->shaders.clear();
    this->programId = -1;
    this->ready = false;
    this->compiled = false;
    this->linked = false;
    this->initialized = false;
}

Program::~Program() {
    this->programId = -1;
    this->ready = false;
    this->compiled = false;
    this->linked = false;
    this->initialized = false;
}

void Program::init() {
    this->programId = glCreateProgram();
    this->initialized = true;
}

void Program::addShader(Shader *shader, GLuint type) {
    try {
        if (shader==nullptr)
            throw std::runtime_error("shader pointer is null.");
        this->shaders.emplace_back(std::make_pair(shader,type));
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error("[Program][Add Shader] "+std::string(e.what()));
    }
}

void Program::compileAll() {
    try {
        for (auto& iter : this->shaders) {
            Shader* shader = iter.first;
            if (shader == nullptr)
                throw std::runtime_error("shader pointer is null.");
            if (!shader->isCompiled()) {
                shader->compile();
                if (!shader->isCompiled()) {
                    throw std::runtime_error("shader compilation failed.");
                }
            }
        }
        this->compiled = true;
    }
    catch (const std::runtime_error& e) {
        throw std::runtime_error("[Program][Compile All]"+std::string(e.what()));
    }
}

void Program::linkAll() {
    try {
        if (!this->initialized)
            throw std::runtime_error("program uninitialized.");
        for (auto& iter : this->shaders) {
            if (!iter.first->isCompiled())
                throw std::runtime_error("shader not compiled");
            glAttachShader(this->programId, iter.first->getId());
        }
        glLinkProgram(this->programId);
        GLint success;
        glGetProgramiv(this->programId, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(this->programId, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error("link failed: " + std::string(infoLog));
        }
        this->linked = true;
    }
    catch (std::runtime_error& e) {
        throw std::runtime_error("[Program][Link All]"+std::string(e.what()));
    }
}

bool Program::checkReady() {
    return this->ready = this->compiled&&this->linked&&this->initialized;
}

void Program::use() {
    glUseProgram(this->programId);
}