/*
* filename：FragmentShader.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#ifndef INITIALGL_FRAGMENTSHADER_H
#define INITIALGL_FRAGMENTSHADER_H

#include "glm.hpp"
//#include "GL/eglew.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "Shader.h"

class FragmentShader :public Shader{
    GLuint fshID;
    const char* fshSrc;
    bool compileSuccess;
public:
    /*Create a fragment shader object with the file path given by user*/
    explicit FragmentShader(const char* vertexPath);
    ~FragmentShader() override;
    void compile() override;
    [[nodiscard]] GLuint getId() override;
    [[nodiscard]] bool isCompiled() override;
};


#endif //INITIALGL_FRAGMENTSHADER_H