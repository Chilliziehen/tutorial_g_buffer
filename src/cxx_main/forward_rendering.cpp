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
#include "common/util/Utilities.h" 

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>              
#include <SceneImporter_Assimp.h>   
#include <data_structures/RenderScene.h>

// 窗口配置
#define WIDTH 1280
#define HEIGHT 1024
#define WINDOW_TITLE "Forward Rendering Experiment"

// 摄像机指针
static Camera* cam = nullptr;
// 键盘状态数组 
static bool g_keys[1024]{};

// 用于计算每帧鼠标的移动量 (Delta X, Delta Y)
float offset_x = 0.0f;
float offset_y = 0.0f;
static float g_lastX = WIDTH * 0.5f;
static float g_lastY = HEIGHT * 0.5f;
static bool g_firstMouse = true;

//键盘回调函数：当用户按下键盘时由 GLFW 调用
static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (key < 0 || key >= 1024) return;
    if (action == GLFW_PRESS) {
        g_keys[key] = true;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE); // ESC 退出
    } else if (action == GLFW_RELEASE) {
        g_keys[key] = false;
    }
}
// 鼠标移动回调函数：计算视角旋转
static void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (g_firstMouse) {
        g_lastX = static_cast<float>(xpos);
        g_lastY = static_cast<float>(ypos);
        g_firstMouse = false;
        return;
    }
    // 计算当前帧与上一帧的鼠标位置差值
    offset_x = static_cast<float>(g_lastX - xpos);
    offset_y = static_cast<float>(g_lastY - ypos);
    g_lastX = static_cast<float>(xpos);
    g_lastY = static_cast<float>(ypos);
}

// 灯光结构体
struct pointLight{
    glm::vec3 position;  // 光源位置
    glm::vec3 color;     // 光源颜色 (RGB)
    float constant;      // 衰减常数项
    float linear;        // 衰减一次项
    float quadratic;     // 衰减二次项
};

/**
 * 【辅助函数】上传材质参数到 Shader
 * * 作用：将 CPU 端的材质数据 (颜色、纹理、粗糙度等) 传递给 GPU 端的 Uniform 变量。
 * 核心概念：
 * 1. Uniform: Shader 中的全局变量。
 * 2. Texture Unit (纹理单元): GPU 有多个纹理插槽 (GL_TEXTURE0, 1, 2...)，
 * 我们需要把纹理对象绑定到插槽，并告诉 Shader 去哪个插槽读取数据。
 */
void setMaterialUniform(const Program& program,
                        const scene::Material& mat,
                        const std::unordered_map<uint32_t, uint32_t>& textureID_map_glTexHandle,
                        const scene::RenderScene& rs) 
{
    GLuint shaderID = program.getId();

    // --- 1. 上传基础材质参数 (PBR 参数) ---
    Utilities::setVec4(shaderID, "uMaterial.baseColorFactor", mat.baseColorFactor);
    Utilities::setFloat(shaderID, "uMaterial.metallicFactor", mat.metallicFactor);
    Utilities::setFloat(shaderID, "uMaterial.roughnessFactor", mat.roughnessFactor);
    // doubleSided 决定是否渲染背面 (0=单面, 1=双面)
    Utilities::setInt(shaderID, "uMaterial.doubleSided", mat.doubleSided ? 1 : 0);

    // 绑定纹理到固定槽位
    // 约定：
    // Slot 0: 漫反射/基色 (Albedo)
    // Slot 1: 法线贴图 (Normal)
    // Slot 2: 高光/金属粗糙度 (Specular)
    // Slot 3: 环境光遮蔽 (AO)
    // Slot 4: 自发光 (Emissive)

    // [Slot 0] Albedo Map
    int idxAlbedo = -1;
    if (mat.albedoMapIndex >= 0) {
        glActiveTexture(GL_TEXTURE0); // 激活 0 号纹理单元
        uint32_t texID = rs.textures[mat.albedoMapIndex].getID();
        glBindTexture(GL_TEXTURE_2D, textureID_map_glTexHandle.at(texID)); // 绑定实际纹理
        
        Utilities::setInt(shaderID, "uTexAlbedo", 0); // 告诉 Shader: uTexAlbedo 对应 0 号槽
        idxAlbedo = 0; // 标记: 有纹理
    }
    Utilities::setInt(shaderID, "uMaterial.albedoTexture", idxAlbedo);

    // [Slot 1] Normal Map
    int idxNormal = -1;
    if (mat.normalMapIndex >= 0) {
        glActiveTexture(GL_TEXTURE1); // 激活 1 号纹理单元
        uint32_t texID = rs.textures[mat.normalMapIndex].getID();
        glBindTexture(GL_TEXTURE_2D, textureID_map_glTexHandle.at(texID));
        
        Utilities::setInt(shaderID, "uTexNormal", 1);
        idxNormal = 1;
    }
    Utilities::setInt(shaderID, "uMaterial.normalTexture", idxNormal);

    // [Slot 2] Specular / MetallicRoughness Map
    int idxSpec = -1;
    if (mat.metallicRoughnessMapIndex >= 0) {
        glActiveTexture(GL_TEXTURE2);
        uint32_t texID = rs.textures[mat.metallicRoughnessMapIndex].getID();
        glBindTexture(GL_TEXTURE_2D, textureID_map_glTexHandle.at(texID));
        
        Utilities::setInt(shaderID, "uTexSpec", 2);
        idxSpec = 2;
    }
    Utilities::setInt(shaderID, "uMaterial.metallicRoughnessTexture", idxSpec);

    // [Slot 3] AO Map
    int idxAO = -1;
    if (mat.aoMapIndex >= 0) {
        glActiveTexture(GL_TEXTURE3);
        uint32_t texID = rs.textures[mat.aoMapIndex].getID();
        glBindTexture(GL_TEXTURE_2D, textureID_map_glTexHandle.at(texID));
        
        Utilities::setInt(shaderID, "uTexAO", 3);
        idxAO = 3;
    }
    Utilities::setInt(shaderID, "uMaterial.aoTexture", idxAO);

    // [Slot 4] Emissive Map
    int idxEmissive = -1;
    if (mat.emissiveMapIndex >= 0) {
        glActiveTexture(GL_TEXTURE4);
        uint32_t texID = rs.textures[mat.emissiveMapIndex].getID();
        glBindTexture(GL_TEXTURE_2D, textureID_map_glTexHandle.at(texID));
        
        Utilities::setInt(shaderID, "uTexEmissive", 4);
        idxEmissive = 4;
    }
    Utilities::setInt(shaderID, "uMaterial.emissiveTexture", idxEmissive);
}


int main() {
    try {
        // 初始化OpenGL上下文(GLFW)
        if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
        
        // 设置OpenGL版本
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // 创建摄像机对象
        cam = new Camera();

        // 创建窗口
        GLFWwindow* hWindow = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
        if (!hWindow) throw std::runtime_error("Failed to create window");
        
        // 设置当前上下文并绑定回调函数
        glfwMakeContextCurrent(hWindow);
        glfwSetKeyCallback(hWindow, key_callback);
        glfwSetCursorPosCallback(hWindow, mouse_callback);
        glfwSetInputMode(hWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 隐藏鼠标光标

        // 初始化GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to init GLEW");

        // 设置视口大小和深度测试
        int viewW, viewH;
        glfwGetFramebufferSize(hWindow, &viewW, &viewH);
        glViewport(0, 0, viewW, viewH);
        glEnable(GL_DEPTH_TEST); // 开启深度测试 (Z-Buffer)，防止透视错误

        scene::SceneImporter_Assimp importer;
        // 加载 glTF 模型文件
        if (!importer.loadScene(std::string(RESOURCES_DIR) + "/example1/lieutenantHead.gltf")) {
            throw std::runtime_error("Failed to load scene");
        }
        scene::RenderScene* rs = &importer.getImportedScene();

        // 场景图 (Scene Graph) 是树状结构，子节点位置依赖父节点。
        // 此函数遍历树结构，预计算出每个节点的绝对世界坐标变换矩阵 (Global Transform)。
        Utilities::calculateGlobalTransforms(rs);

        // 加载并创建纹理对象
        // 将磁盘上的图片加载到显存中，并生成 OpenGL ID
        std::unordered_map<uint32_t, uint32_t> textureID_map_glTexHandle;
        for (const scene::TextureInfo& ti : rs->textures) {
            uint32_t texHandle = Utilities::createTexture2DFromFile(ti.path);
            textureID_map_glTexHandle[ti.getID()] = texHandle;
        }

        // VBO: 顶点缓冲对象 (存储位置、法线、UV等数据)
        // EBO: 索引缓冲对象 (存储三角形的顶点索引)
        // VAO: 顶点数组对象 (记录数据格式布局)
        GLuint vao, vbo, ebo;
        glGenVertexArrays(1, &vao); glBindVertexArray(vao);
        
        // 上传所有顶点数据
        glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, rs->globalVertices.size() * sizeof(scene::Vertex), rs->globalVertices.data(), GL_STATIC_DRAW);
        
        // 上传所有索引数据
        glGenBuffers(1, &ebo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, rs->globalIndices.size() * sizeof(uint32_t), rs->globalIndices.data(), GL_STATIC_DRAW);

        // 告诉OpenGL如何解析顶点数据结构
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, position));
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, normal));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, texCoord));
        glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, tangent));
        glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, bitangent));
        
        glBindVertexArray(0); // 解绑 VAO

        // 编译着色器程序
        Program forwardProg;
        forwardProg.init();
        // 加载并编译 .vert (顶点着色器) 和 .frag (片段着色器)
        forwardProg.addShader(new VertexShader((std::string(GLSL_ROOT) + "/fwd.vert").c_str()), GL_VERTEX_SHADER);
        forwardProg.addShader(new FragmentShader((std::string(GLSL_ROOT) + "/fwd.frag").c_str()), GL_FRAGMENT_SHADER);
        forwardProg.compileAll();
        forwardProg.linkAll(); // 链接成完整的 Program

        forwardProg.use();

        // 渲染glTF scene的方式:dfs
        std::vector<uint32_t> nodeRenderStack;
        std::vector<uint32_t> dfsStack;
        dfsStack.push_back(rs->rootIndex);
        while (!dfsStack.empty()) {
            uint32_t current = dfsStack.back();
            dfsStack.pop_back();
            nodeRenderStack.push_back(current);
            const scene::Node& node = rs->nodes[current];
            for (int i : node.children) dfsStack.push_back(i);
        }

        cam->cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // 初始摄像机位置
        
        // 设置一个点光源
        pointLight pl;
        pl.position = glm::vec3(-5.0f, 2.0f, -5.0f);
        pl.color = glm::vec3(5.0f, 5.0f, 5.0f); 
        pl.constant = 1.0f;
        pl.linear = 0.09f;      
        pl.quadratic = 0.032f;

        // 将灯光参数上传到 Shader (这些参数每帧不变，可以循环外上传)
        int32_t locPointLightPos = glGetUniformLocation(forwardProg.getId(), "uPointLight.position");
        int32_t locPointLightColor = glGetUniformLocation(forwardProg.getId(), "uPointLight.color");
        int32_t locPointLightConstant = glGetUniformLocation(forwardProg.getId(), "uPointLight.constant");
        int32_t locPointLightLinear = glGetUniformLocation(forwardProg.getId(), "uPointLight.linear");
        int32_t locPointLightQuadratic = glGetUniformLocation(forwardProg.getId(), "uPointLight.quadratic");
        
        glUniform3fv(locPointLightPos, 1, glm::value_ptr(pl.position));
        glUniform3fv(locPointLightColor, 1, glm::value_ptr(pl.color));
        glUniform1f(locPointLightConstant, pl.constant);
        glUniform1f(locPointLightLinear, pl.linear);
        glUniform1f(locPointLightQuadratic, pl.quadratic);

        // 渲染主循环(每一帧执行一次)
        while (!glfwWindowShouldClose(hWindow)) {
            // 处理输入
            glfwPollEvents();
            cam->move(g_keys);
            cam->mousemove();
            cam->refresh();

            // 清除屏幕
            int fbW, fbH;
            glfwGetFramebufferSize(hWindow, &fbW, &fbH);
            glViewport(0, 0, fbW, fbH);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 背景色 (深灰)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色缓冲和深度缓冲

            forwardProg.use();
            Utilities::setVec3(forwardProg.getId(), "uViewPos", cam->cameraPos); 
            
            // 计算VP矩阵
            glm::mat4 view = cam->getViewMatrix();
            glm::mat4 proj = glm::perspective(45.0f, (float)fbW/fbH, 0.1f, 1000.0f);
            Utilities::setMat4(forwardProg.getId(), "uView", view);
            Utilities::setMat4(forwardProg.getId(), "uProj", proj);

            glBindVertexArray(vao); // 绑定几何数据

            // 遍历场景节点并绘制
            for (auto nodeIdx : nodeRenderStack) {
                const scene::Node& node = rs->nodes[nodeIdx];
                if (node.meshIndex < 0) continue; 

                glm::mat4 model = node.globalTransform.has_value() ? node.globalTransform.value() : glm::mat4(1.0f);
                Utilities::setMat4(forwardProg.getId(), "uModel", model);

                const scene::Mesh& mesh = rs->meshes[node.meshIndex];
                for (const auto& submesh : mesh.subMeshes) {
                    const scene::Material& mat = rs->materials[submesh.materialIndex];
                    // 上传材质参数和绑定纹理
                    setMaterialUniform(forwardProg, mat, textureID_map_glTexHandle, *rs);
                    // 执行绘制命令
                    glDrawElements(GL_TRIANGLES, 
                                static_cast<GLsizei>(submesh.indexCount), 
                                GL_UNSIGNED_INT, 
                                reinterpret_cast<void*>(submesh.baseIndex * sizeof(uint32_t)));
                }
            }
            glBindVertexArray(0); // 解绑 VAO
            glfwSwapBuffers(hWindow);
        }
        
        delete cam;
        glfwTerminate();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}