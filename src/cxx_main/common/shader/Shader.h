/*
* filename：Shader.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#ifndef INITIALGL_SHADER_H
#define INITIALGL_SHADER_H
#include "GL/glew.h"


class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile() = 0;
    virtual GLuint getId() = 0;
    virtual bool isCompiled() = 0;
};


#endif //INITIALGL_SHADER_H