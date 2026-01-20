/*
* filename：VertexShader.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#ifndef INITIALGL_VERTEXSHADER_H
#define INITIALGL_VERTEXSHADER_H

#include "glm.hpp"
//#include "GL/eglew.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "Shader.h"

class VertexShader :public Shader{
    GLuint vshID;
    const char* vshSrc;
    bool compileSuccess;
public:
    /*Create a vertex shader object with the file path given by user*/
    explicit VertexShader(const char* vertexPath);
    ~VertexShader() override;
    void compile() override;
    [[nodiscard]] GLuint getId() override ;
    [[nodiscard]] bool isCompiled() override ;
};

#endif //INITIALGL_VERTEXSHADER_H