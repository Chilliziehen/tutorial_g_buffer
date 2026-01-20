/*
* filename：Program.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#ifndef INITIALGL_PROGRAM_H
#define INITIALGL_PROGRAM_H
#include <vector>
#include "../shader/Shader.h"
#include "glm.hpp"
//#include "GL/eglew.h"
#include "GLFW/glfw3.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
class Program {
    std::vector<std::pair<Shader*,GLuint>> shaders;
    GLuint programId;
    bool initialized;
    bool compiled;
    bool linked;
    bool ready;
public:
    explicit Program();
    virtual ~Program();
    void addShader(Shader* shader,GLuint type);
    void compileAll();
    void linkAll();
    void use();
    void init();
    bool checkReady();
    [[nodiscard]] inline GLuint getId() const { return programId; }
};


#endif //INITIALGL_PROGRAM_H