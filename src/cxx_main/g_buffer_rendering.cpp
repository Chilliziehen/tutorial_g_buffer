/*
* filename：g_buffer_rendering
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "config/config.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "common/shader/FragmentShader.h"
#include "common/shader/VertexShader.h"
#include "common/camera/Camera.h"
#include "common/program/Program.h"
#include "common/light/LightManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <queue>
#include <stb_image.h>

#include <SceneImporter_Assimp.h>
#include <data_structures/RenderScene.h>
#include <SceneRenderer_GL.h>

#include "common/util/Utilities.h"

#define WIDTH 1280
#define HEIGHT 1024
#define WINDOW_TITLE "G-Buffer"

static Camera* cam = nullptr;

static bool g_keys[1024]{};
static bool g_firstMouse = true;
static double g_lastX = WIDTH * 0.5;
static double g_lastY = HEIGHT * 0.5;
static float g_deltaTime = 0.0f;
static float g_lastFrame = 0.0f;

static int g_debugMode = 3; // 0=lit,1=pos,2=normal,3=albedo (default show albedo)

// 全文件常量：避免作用域问题
#define MAX_TEXTURES 32
#define MAX_MATERIALS 32

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key < 0 || key >= 1024) return;
    if (action == GLFW_PRESS) {
        g_keys[key] = true;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_1) g_debugMode = 0;
        if (key == GLFW_KEY_2) g_debugMode = 1;
        if (key == GLFW_KEY_3) g_debugMode = 2;
        if (key == GLFW_KEY_4) g_debugMode = 3;
        if (key == GLFW_KEY_5) g_debugMode = 5; // depth debug
    } else if (action == GLFW_RELEASE) {
        g_keys[key] = false;
    }
}

// 这些变量由 GLFW 回调写入，Camera::mousemove 读取（在 Camera.h/cpp 里以 extern 引用）。
GLfloat offset_x = 0.0f, offset_y = 0.0f;
static GLfloat lastx = 0.0f, lasty = 0.0f;
static bool firstMouse = true;
float scale = 1.0f;

static void mouse_callback(GLFWwindow* hwnd, double xpos, double ypos) {
    //Mouse handling logic
    if (firstMouse) {
        lastx = static_cast<GLfloat>(xpos);
        lasty = static_cast<GLfloat>(ypos);
        firstMouse = false;
        offset_x = 0.0f;
        offset_y = 0.0f;
        return;
    }
    offset_x = static_cast<GLfloat>(lastx - xpos);
    offset_y = static_cast<GLfloat>(lasty - ypos); // Reversed since y-coordinates go from bottom
    lastx = static_cast<GLfloat>(xpos);
    lasty = static_cast<GLfloat>(ypos);
    cam->refresh();
}
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //Scroll handling logic
    scale += yoffset;
}

void setMaterialUniform(const Program& program,
    const scene::Material& mat,
    const std::unordered_map<uint32_t,uint32_t>& textureID_map_glTexHandle,
    const scene::RenderScene& rs,
    const uint32_t materialIndex) {
    struct materialUniformDTO {
        glm::vec4 baseColorFactor;
        float metallicFactor;
        float roughnessFactor;
        int32_t doubleSided;
        int32_t albedoTexture = -1;
        int32_t normalTexture = -1;
        int32_t metallicRoughnessTexture = -1;
        int32_t aoTexture = -1;
        int32_t emissiveTexture = -1;
    } dto;
    static int32_t cumTextureSlot = 0;
    static int32_t baseOffset = 5;
    //准备Data Transfer Object
    dto.baseColorFactor = mat.baseColorFactor;
    dto.metallicFactor = mat.metallicFactor;
    dto.roughnessFactor = mat.roughnessFactor;
    dto.doubleSided = mat.doubleSided ? 1 : 0;
    //int32_t loc = glGetUniformLocation(program.getId(), ("materials[" + std::to_string(materialIndex) + "]").c_str());
    if (mat.albedoMapIndex>=0) {
        dto.albedoTexture = cumTextureSlot++;
        //激活单元
        glActiveTexture(GL_TEXTURE0 + baseOffset + dto.albedoTexture);
        const scene::TextureInfo ti = rs.textures[mat.albedoMapIndex];
        uint32_t textureHandle = textureID_map_glTexHandle.at(ti.getID());
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        //绑定采样器
        int32_t textureLoc = glGetUniformLocation(program.getId(), ("uTextures[" + std::to_string(dto.albedoTexture) + "]").c_str());
        glUniform1i(textureLoc, baseOffset + dto.albedoTexture);
    }
    if (mat.normalMapIndex>=0) {
        dto.normalTexture = cumTextureSlot++;
        glActiveTexture(GL_TEXTURE0 + baseOffset + dto.normalTexture);
        const scene::TextureInfo ti = rs.textures[mat.normalMapIndex];
        uint32_t textureHandle = textureID_map_glTexHandle.at(ti.getID());
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        //绑定采样器
        int32_t textureLoc = glGetUniformLocation(program.getId(), ("uTextures[" + std::to_string(dto.normalTexture) + "]").c_str());
        glUniform1i(textureLoc, baseOffset + dto.normalTexture);
    }
    if (mat.metallicRoughnessMapIndex>=0) {
        dto.metallicRoughnessTexture = cumTextureSlot++;
        glActiveTexture(GL_TEXTURE0 + baseOffset + dto.metallicRoughnessTexture);
        const scene::TextureInfo ti = rs.textures[mat.metallicRoughnessMapIndex];
        uint32_t textureHandle = textureID_map_glTexHandle.at(ti.getID());
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        //绑定采样器
        int32_t textureLoc = glGetUniformLocation(program.getId(), ("uTextures[" + std::to_string(dto.metallicRoughnessTexture) + "]").c_str());
        glUniform1i(textureLoc, baseOffset + dto.metallicRoughnessTexture);
    }
    if (mat.aoMapIndex>=0) {
        dto.aoTexture = cumTextureSlot++;
        glActiveTexture(GL_TEXTURE0 + baseOffset + dto.aoTexture);
        const scene::TextureInfo ti = rs.textures[mat.aoMapIndex];
        uint32_t textureHandle = textureID_map_glTexHandle.at(ti.getID());
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        //绑定采样器
        int32_t textureLoc = glGetUniformLocation(program.getId(), ("uTextures[" + std::to_string(dto.aoTexture) + "]").c_str());
        glUniform1i(textureLoc, baseOffset + dto.aoTexture);
    }
    if (mat.emissiveMapIndex>=0) {
        dto.emissiveTexture = cumTextureSlot++;
        glActiveTexture(GL_TEXTURE0 + baseOffset + dto.emissiveTexture);
        const scene::TextureInfo ti = rs.textures[mat.emissiveMapIndex];
        uint32_t textureHandle = textureID_map_glTexHandle.at(ti.getID());
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        //绑定采样器
        int32_t textureLoc = glGetUniformLocation(program.getId(), ("uTextures[" + std::to_string(dto.emissiveTexture) + "]").c_str());
        glUniform1i(textureLoc, baseOffset + dto.emissiveTexture);
    }
    //上传material uniform
    int32_t locBaseColorFactor = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].baseColorFactor").c_str());
    int32_t locMetallicFactor = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].metallicFactor").c_str());
    int32_t locRoughnessFactor = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].roughnessFactor").c_str());
    int32_t locDoubleSided = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].doubleSided").c_str());
    int32_t locAlbedoTexture = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].albedoTexture").c_str());
    int32_t locNormalTexture = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].normalTexture").c_str());
    int32_t locMetallicRoughnessTexture = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].metallicRoughnessTexture").c_str());
    int32_t locAOTexture = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].aoTexture").c_str());
    int32_t locEmissiveTexture = glGetUniformLocation(program.getId(), ("uMaterials[" + std::to_string(materialIndex) + "].emissiveTexture").c_str());
    glUniform4fv(locBaseColorFactor, 1, glm::value_ptr(dto.baseColorFactor));
    glUniform1f(locMetallicFactor, dto.metallicFactor);
    glUniform1f(locRoughnessFactor, dto.roughnessFactor);
    glUniform1i(locDoubleSided, dto.doubleSided);
    glUniform1i(locAlbedoTexture, dto.albedoTexture);
    glUniform1i(locNormalTexture, dto.normalTexture);
    glUniform1i(locMetallicRoughnessTexture, dto.metallicRoughnessTexture);
    glUniform1i(locAOTexture, dto.aoTexture);
    glUniform1i(locEmissiveTexture, dto.emissiveTexture);
}

struct pointLight{
    glm::vec3 position;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
};

int main() {
    try {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to init GLFW");
        }
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        cam = new Camera();

        GLFWwindow* hWindow = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
        if (hWindow == nullptr) {
            throw std::runtime_error("Failed to create GLFW window");
        }
        glfwMakeContextCurrent(hWindow);
        glfwSetKeyCallback(hWindow, key_callback);
        glfwSetCursorPosCallback(hWindow, mouse_callback);
        glfwSetInputMode(hWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        int viewW = WIDTH;
        int viewH = HEIGHT;
        glfwGetFramebufferSize(hWindow, &viewW, &viewH);
        glViewport(0, 0, viewW, viewH);
        glEnable(GL_DEPTH_TEST);

        scene::RenderScene* rs;
        scene::SceneImporter_Assimp importer;
        if (!importer.loadScene(std::string(RESOURCES_DIR) + "/example1/lieutenantHead.gltf")) {
            throw std::runtime_error("Failed to load scene");
        }
        rs = &importer.getImportedScene();
        Utilities::calculateGlobalTransforms(rs);

        //通过TextureInfo.id映射纹理句柄
        std::unordered_map<uint32_t,uint32_t> textureID_map_glTexHandle;
        for (const scene::TextureInfo& ti: rs->textures) {
            uint32_t texHandle = Utilities::createTexture2DFromFile(ti.path);
            textureID_map_glTexHandle[ti.getID()] = texHandle;
        }
        uint32_t gbuffer;
        glGenFramebuffers(1, &gbuffer);
        uint32_t gPosition, gNormal, gUV,gMaterialIndex;

        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewW, viewH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewW, viewH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D, gNormal, 0);

        glGenTextures(1, &gUV);
        glBindTexture(GL_TEXTURE_2D, gUV);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, viewW, viewH, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D, gUV, 0);

        glGenTextures(1, &gMaterialIndex);
        glBindTexture(GL_TEXTURE_2D, gMaterialIndex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, viewW, viewH, 0, GL_RED_INTEGER, GL_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D, gMaterialIndex, 0);

        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);
        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewW, viewH);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("G-buffer framebuffer not complete");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const glm::vec2 viewRectangle[] = {
            {0.0f,0.0f},
            {1.0f,0.0f},
            {1.0f,1.0f},
            {0.0f,1.0f}
        };
        const uint32_t viewRectangleIndices[] = {
            0,1,2,
            2,3,0
        };
        GLuint quadVAO, quadVBO, quadEBO;
        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(viewRectangle), viewRectangle, GL_STATIC_DRAW);
        glGenBuffers(1, &quadEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(viewRectangleIndices), viewRectangleIndices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), reinterpret_cast<void*>(0));
        glBindVertexArray(0);

        std::unordered_map<uint32_t,uint32_t> nodeIndex_map_vao;
        std::vector<uint32_t> nodeRenderStack;
        std::vector<uint32_t> dfsStack;
        dfsStack.push_back(rs->rootIndex);
        while (!dfsStack.empty()) {
            uint32_t current = dfsStack.back();
            dfsStack.pop_back();
            nodeRenderStack.push_back(current);
            const scene::Node& node = rs->nodes[current];
            for (int i : node.children) {
                dfsStack.push_back(i);
            }
        }

        uint32_t vao,vbo,ebo;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, rs->globalVertices.size() * sizeof(scene::Vertex), rs->globalVertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, rs->globalIndices.size() * sizeof(uint32_t), rs->globalIndices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), reinterpret_cast<void*>(offsetof(scene::Vertex, position)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), reinterpret_cast<void*>(offsetof(scene::Vertex, normal)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), reinterpret_cast<void*>(offsetof(scene::Vertex, texCoord)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), reinterpret_cast<void*>(offsetof(scene::Vertex, tangent)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), reinterpret_cast<void*>(offsetof(scene::Vertex, bitangent)));
        glBindVertexArray(0);

        Program gBufferProgram,lightingProgram;
        gBufferProgram.init();
        lightingProgram.init();

        const std::string gBufferVertPath = std::string(GLSL_ROOT) + "/g_buffer_geom.vert";
        const std::string gBufferFragPath = std::string(GLSL_ROOT) + "/g_buffer_geom.frag";
        VertexShader *gBufferVShader = new VertexShader(gBufferVertPath.c_str());
        FragmentShader *gBufferFShader = new FragmentShader(gBufferFragPath.c_str());
        gBufferProgram.addShader(gBufferVShader, GL_VERTEX_SHADER);
        gBufferProgram.addShader(gBufferFShader, GL_FRAGMENT_SHADER);
        gBufferProgram.compileAll();
        gBufferProgram.linkAll();
        if (!gBufferProgram.checkReady())
            throw std::runtime_error("G-Buffer program not ready");

        const std::string lightingVertPath = std::string(GLSL_ROOT) + "/deferred_lighting.vert";
        const std::string lightingFragPath = std::string(GLSL_ROOT) + "/deferred_lighting.frag";
        VertexShader *lightingVShader = new VertexShader(lightingVertPath.c_str());
        FragmentShader *lightingFShader = new FragmentShader(lightingFragPath.c_str());
        lightingProgram.addShader(lightingVShader, GL_VERTEX_SHADER);
        lightingProgram.addShader(lightingFShader, GL_FRAGMENT_SHADER);
        lightingProgram.compileAll();
        lightingProgram.linkAll();
        if (!lightingProgram.checkReady())
            throw std::runtime_error("Lighting program not ready");

        int32_t locModel = glGetUniformLocation(gBufferProgram.getId(), "uModel");
        int32_t locView = glGetUniformLocation(gBufferProgram.getId(), "uView");
        int32_t locProjection = glGetUniformLocation(gBufferProgram.getId(), "uProj");
        int32_t locMaterialIndex = glGetUniformLocation(gBufferProgram.getId(), "uMaterialIndex");

        lightingProgram.use();
        for (int i=0;i<=rs->materials.size()-1;++i) {
            const scene::Material& material = rs->materials[i];
            setMaterialUniform(lightingProgram,material,textureID_map_glTexHandle,*rs,i);
        }

        pointLight pl;
        pl.position = glm::vec3(0.0f, 5.0f, 5.0f);
        pl.color = glm::vec3(1.0f, 1.0f, 1.0f);
        pl.constant = 1.0f;
        pl.linear = 0.09f;
        pl.quadratic = 0.032f;

        int32_t locPointLightPos = glGetUniformLocation(lightingProgram.getId(), "uPointLight.position");
        int32_t locPointLightColor = glGetUniformLocation(lightingProgram.getId(), "uPointLight.color");
        int32_t locPointLightConstant = glGetUniformLocation(lightingProgram.getId(), "uPointLight.constant");
        int32_t locPointLightLinear = glGetUniformLocation(lightingProgram.getId(), "uPointLight.linear");
        int32_t locPointLightQuadratic = glGetUniformLocation(lightingProgram.getId(), "uPointLight.quadratic");
        glUniform3fv(locPointLightPos, 1, glm::value_ptr(pl.position));
        glUniform3fv(locPointLightColor, 1, glm::value_ptr(pl.color));
        glUniform1f(locPointLightConstant, pl.constant);
        glUniform1f(locPointLightLinear, pl.linear);
        glUniform1f(locPointLightQuadratic, pl.quadratic);
        while (true) {
            glfwPollEvents();
            if (glfwWindowShouldClose(hWindow))
                break;
            cam->move(g_keys);
            cam->mousemove();
            cam->refresh();
            gBufferProgram.use();
            glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
            glBindVertexArray(vao);
            glViewport(0, 0, viewW, viewH);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glm::mat4 view = cam->getViewMatrix();
            glm::mat4 projection = glm::perspective(45.0f, static_cast<float>(viewW) / static_cast<float>(viewH), 0.1f, 1000.0f);
            glUniformMatrix4fv(locProjection, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
            //渲染glTF scene的方式:dfs
            for (auto nodeIdx:nodeRenderStack) {
                scene::Node& node = rs->nodes[nodeIdx];
                if (node.meshIndex < 0)
                    continue;
                scene::Mesh& mesh = rs->meshes[node.meshIndex];
                glm::mat4 model;
                if (node.globalTransform.has_value())
                    model = node.globalTransform.value();
                else
                    model = glm::mat4(1.0f);
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                for (const auto& submesh:mesh.subMeshes) {
                    scene::Material& mat = rs->materials[submesh.materialIndex];
                    glBindVertexArray(vao);
                    //设置子网格材质
                    glUniform1i(locMaterialIndex, submesh.materialIndex);
                    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(submesh.indexCount), GL_UNSIGNED_INT, reinterpret_cast<void*>(submesh.baseIndex * sizeof(uint32_t)));
                    glBindVertexArray(0);
                }
            }
            //首先映射material
            lightingProgram.use();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, viewW, viewH);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            Utilities::setInt(lightingProgram.getId(), "gPosition", 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            Utilities::setInt(lightingProgram.getId(), "gNormal", 1);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gUV);
            Utilities::setInt(lightingProgram.getId(), "gUV", 2);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, gMaterialIndex);
            Utilities::setInt(lightingProgram.getId(), "gMaterialIndex", 3);
            glBindVertexArray(quadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glfwSwapBuffers(hWindow);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        if (cam) {
            delete cam;
            cam = nullptr;
        }
        glfwTerminate();
        return 1;
    }
    return 0;
}


