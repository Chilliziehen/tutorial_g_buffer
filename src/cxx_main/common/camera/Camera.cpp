/*
* filename：Camera.cpp
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include "Camera.h"
#include "cmath"

// 鼠标灵敏度（弧度/像素）
static const GLfloat MouseSensitivity = 0.002f;
static const glm::mat4 defaultMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
static const glm::vec3 defaultPosition = glm::vec3(0.0f, 0.0f, 3.0f);
static const glm::vec3 defaultLook = glm::vec3(0.0f, 0.0f, 0.0f);
static const GLfloat defaultSpeed = 0.05f;

// Mouse deltas are defined in the main translation unit (e.g., g_buffer_rendering.cpp)
extern GLfloat offset_x;
extern GLfloat offset_y;
extern float scale;

Camera::Camera()
{
    this->cameraPos = defaultPosition;
    //this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // 面向 -Z
    this->yaw = 0.0f; // -90 度
    this->pitch = 0.0f;
    this->cameraFront = glm::normalize(glm::vec3(
            -std::cos(this->pitch) * std::sin(this->yaw),
             std::sin(this->pitch),
            -std::cos(this->pitch) * std::cos(this->yaw)));
    this->cameraRight = glm::normalize(glm::cross(this->cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));
    this->projectionMatrix = glm::perspective(45.0f,16.0f/9.0f,0.1f,1000.0f);
    this->refresh();
}

Camera::Camera(glm::vec3 position, glm::vec3 directionF, glm::vec3 directionU)
{
    this->cameraPos = position;
    //this->worldUp = glm::normalize(directionU);
    this->cameraFront = glm::normalize(directionF);
    // 从前向量推导 yaw/pitch（保持与弧度一致）
    this->pitch = std::asin(glm::clamp(this->cameraFront.y, -1.0f, 1.0f));
    this->yaw = std::atan2(this->cameraFront.z, this->cameraFront.x);
    this->cameraRight = glm::normalize(glm::cross(this->cameraFront, glm::normalize(directionU)));
    this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));
    this->projectionMatrix = glm::perspective(45.0f,16.0f/9.0f,0.1f,100.0f);
    this->refresh();
}

Camera::~Camera(){}

void Camera::refresh()
{
    // 更新正交基
    //this->cameraRight = glm::normalize(glm::cross(this->cameraFront, this->worldUp));
    //this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));
    this->viewMatrix = glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
}

void Camera::move(bool *keys)
{
    GLfloat speed = defaultSpeed;
    if(keys[GLFW_KEY_LEFT_SHIFT])
    {
        speed *= 0.4f;
    }
    if(keys[GLFW_KEY_W])
    {
        this->cameraPos += speed * this->cameraFront;
    }
    if(keys[GLFW_KEY_S])
    {
        this->cameraPos -= speed * this->cameraFront;
    }
    if(keys[GLFW_KEY_A])
    {
        this->cameraPos -= speed * this->cameraRight;
    }
    if(keys[GLFW_KEY_D])
    {
        this->cameraPos += speed * this->cameraRight;
    }
    this->refresh();
}


void Camera::mousemove()
{
    this->yaw   += offset_x * MouseSensitivity;
    this->pitch += offset_y * MouseSensitivity;

    // 限制俯仰角避免万向锁
    const float limit = glm::radians(89.0f);
    if(this->pitch > limit)
        this->pitch = limit;
    if(this->pitch < -limit)
        this->pitch = -limit;

    // 根据 yaw/pitch 重建前向（右手坐标系）
    this->cameraFront = glm::normalize(glm::vec3(
            -std::cos(this->pitch) * std::sin(this->yaw),
             std::sin(this->pitch),
            -std::cos(this->pitch) * std::cos(this->yaw)));
    this->cameraRight = glm::normalize(glm::vec3(
        std::cos(yaw),
        0.0f,
        -std::sin(yaw)));
    this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));

    // 清空增量并刷新矩阵
    offset_x = 0.0f;
    offset_y = 0.0f;
    this->refresh();
}

glm::mat4 Camera::getViewMatrix()
{
    return this->viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix()
{
    return this->projectionMatrix;
}

const glm::mat4& Camera::setProjectionMatrix(const glm::mat4 &project)
{
    return this->projectionMatrix = project;
}