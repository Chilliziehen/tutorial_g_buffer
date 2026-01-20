/*
* filename：Camera.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#ifndef INC_2025_AUTUMN_CG_CAMERA_H
#define INC_2025_AUTUMN_CG_CAMERA_H
#include <glm.hpp>
//#include "GL/eglew.h"
#include <GLFW/glfw3.h>
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
class Camera {
private:

public:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    //glm::vec3 worldUp;   // 世界上方向
    GLfloat yaw;
    GLfloat pitch;
    /*The first parameter tells the position of the camera,
    and the second one tells the direction of the camera.*/
    Camera(glm::vec3, glm::vec3,glm::vec3);
    Camera();
    /*If user didn't give Camera parameters,
     *Initialize the object's matrix with default view matrix.*/
    ~Camera();
    /*Deconstruct the camera object*/
    glm::mat4 getViewMatrix();
    void refresh();
    void move(bool *keys);
    void mousemove();
    const glm::mat4& setProjectionMatrix(const glm::mat4 &project);
    const glm::mat4& getProjectionMatrix();
};


#endif //INC_2025_AUTUMN_CG_CAMERA_H