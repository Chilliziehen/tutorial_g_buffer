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
#include <stb_image.h>

#include <SceneImporter_Assimp.h>
#include <data_structures/RenderScene.h>

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

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key < 0 || key >= 1024) return;
    if (action == GLFW_PRESS) {
        g_keys[key] = true;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_1) g_debugMode = 0;
        if (key == GLFW_KEY_2) g_debugMode = 1;
        if (key == GLFW_KEY_3) g_debugMode = 2;
        if (key == GLFW_KEY_4) g_debugMode = 3;
    } else if (action == GLFW_RELEASE) {
        g_keys[key] = false;
    }
}
extern GLfloat offset_x, offset_y;
extern GLfloat lastx = 0.0f, lasty = 0.0f;
extern bool firstMouse = true;
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

struct GBuffer {
    GLuint fbo = 0;
    GLuint gPosition = 0;
    GLuint gNormal = 0;
    GLuint gAlbedoSpec;
    GLuint rboDepth = 0;
    int width = 0;
    int height = 0;
};

static void destroyGBuffer(GBuffer& g) {
    if (g.rboDepth) glDeleteRenderbuffers(1, &g.rboDepth);
    if (g.gPosition) glDeleteTextures(1, &g.gPosition);
    if (g.gNormal) glDeleteTextures(1, &g.gNormal);
    if (g.gAlbedoSpec) glDeleteTextures(1, &g.gAlbedoSpec);
    if (g.fbo) glDeleteFramebuffers(1, &g.fbo);
    g = {};
}

static void createGBuffer(GBuffer& g, int width, int height) {
    destroyGBuffer(g);
    g.width = width;
    g.height = height;

    glGenFramebuffers(1, &g.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g.fbo);

    auto makeColorTex = [&](GLuint& outTex, GLenum internalFmt, GLenum fmt, GLenum type, GLenum attachment) {
        glGenTextures(1, &outTex);
        glBindTexture(GL_TEXTURE_2D, outTex);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, width, height, 0, fmt, type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, outTex, 0);
    };
    makeColorTex(g.gPosition, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    makeColorTex(g.gNormal, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1);
    makeColorTex(g.gAlbedoSpec, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    glGenRenderbuffers(1, &g.rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, g.rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g.rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw std::runtime_error("G-buffer framebuffer not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

        // Fullscreen pass needs a VAO in core profile
        GLuint fsVao = 0;
        glGenVertexArrays(1, &fsVao);

        // ----- Load assets (glTF) -----
        scene::SceneImporter_Assimp importer;
        const std::string scenePath = std::string(RESOURCES_DIR) + "/example1/lieutenantHead.gltf";
        if (!importer.loadScene(scenePath)) {
            throw std::runtime_error("Failed to load scene: " + scenePath);
        }
        scene::RenderScene& rs = importer.getImportedScene();
        if (rs.globalVertices.empty() || rs.globalIndices.empty() || rs.rootIndex < 0) {
            throw std::runtime_error("Imported scene is empty/invalid.");
        }
        //rs.nodes[0].localTransform.scale = glm::vec3(0.5f,0.5f,0.5f);
        // ----- Upload pooled mesh buffers -----
        GLuint vao = 0, vbo = 0, ebo = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(rs.globalVertices.size() * sizeof(scene::Vertex)),
                     rs.globalVertices.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(rs.globalIndices.size() * sizeof(uint32_t)),
                     rs.globalIndices.data(),
                     GL_STATIC_DRAW);

        // layout matches scene::Vertex
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, texCoord));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(scene::Vertex), (void*)offsetof(scene::Vertex, bitangent));

        glBindVertexArray(0);

        // ----- Load textures (albedo only) -----
        std::unordered_map<std::string, GLuint> texCache;
        std::unordered_map<int, GLuint> albedoTexByMaterial;
        for (size_t mi = 0; mi < rs.materials.size(); ++mi) {
            const scene::Material& m = rs.materials[mi];
            if (m.albedoMapIndex < 0 || static_cast<size_t>(m.albedoMapIndex) >= rs.textures.size()) continue;
            const std::string& path = rs.textures[m.albedoMapIndex].path;
            if (path.empty() || (!path.empty() && path[0] == '*')) continue; // skip embedded

            GLuint tex = 0;
            const auto it = texCache.find(path);
            if (it != texCache.end()) {
                tex = it->second;
            } else {
                tex = createTexture2DFromFile(path);
                texCache.emplace(path, tex);
            }
            albedoTexByMaterial.emplace(static_cast<int>(mi), tex);
        }

        // ----- Build programs -----
        Program gbufProg;
        gbufProg.init();
        const std::string gbufVertGeomPath = std::string(GLSL_ROOT) + "/g_buffer_geom.vert";
        const std::string gbufFragGeomPath = std::string(GLSL_ROOT) + "/g_buffer_geom.frag";
        gbufProg.addShader(new VertexShader(gbufVertGeomPath.c_str()), GL_VERTEX_SHADER);
        gbufProg.addShader(new FragmentShader(gbufFragGeomPath.c_str()), GL_FRAGMENT_SHADER);
        gbufProg.compileAll();
        gbufProg.linkAll();
        if (!gbufProg.checkReady()) throw std::runtime_error("G-buffer program not ready");

        Program lightProg;
        lightProg.init();
        const std::string lightingVertPath = std::string(GLSL_ROOT) + "/deferred_lighting.vert";
        const std::string lightingFragPath = std::string(GLSL_ROOT) + "/deferred_lighting.frag";
        lightProg.addShader(new VertexShader(lightingVertPath.c_str()), GL_VERTEX_SHADER);
        lightProg.addShader(new FragmentShader(lightingFragPath.c_str()), GL_FRAGMENT_SHADER);
        lightProg.compileAll();
        lightProg.linkAll();
        if (!lightProg.checkReady()) throw std::runtime_error("Lighting program not ready");

        // ----- G-buffer -----
        GBuffer gbuf;
        createGBuffer(gbuf, viewW, viewH);

        // ----- Lights -----
        LightManager lights;
        lights.add(Light(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(1.0f, 0.95f, 0.9f)));
        lights.add(Light(glm::vec3(-3.0f, 2.0f, 1.0f), glm::vec3(0.6f, 0.7f, 1.0f)));

        // Default camera placement
        cam->cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
        cam->yaw = -90.0f;
        cam->pitch = 0.0f;
        cam->refresh();

        // ----- Main loop -----
        while (!glfwWindowShouldClose(hWindow)) {
            const float now = static_cast<float>(glfwGetTime());
            g_deltaTime = now - g_lastFrame;
            g_lastFrame = now;

            glfwPollEvents();
            cam->move(g_keys);
            cam->mousemove();
            cam->refresh();
            int fbW = 0, fbH = 0;
            glfwGetFramebufferSize(hWindow, &fbW, &fbH);
            if (fbW != gbuf.width || fbH != gbuf.height) {
                createGBuffer(gbuf, fbW, fbH);
            }
            const glm::mat4 view = cam->getViewMatrix();
            const glm::mat4 proj = cam->setProjectionMatrix(glm::perspective(45.0f,
                                                                             static_cast<float>(fbW) / static_cast<float>(fbH),
                                                                             0.1f, 2000.0f));
            // 1) Geometry pass
            glBindFramebuffer(GL_FRAMEBUFFER, gbuf.fbo);
            glViewport(0, 0, gbuf.width, gbuf.height);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glBindVertexArray(vao);
            gbufProg.use();
            setMat4(gbufProg.getId(), "uView", view);
            setMat4(gbufProg.getId(), "uProj", proj);
            drawSceneNode(rs, rs.rootIndex, glm::mat4(1.0f), gbufProg.getId(), albedoTexByMaterial);
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // 2) Lighting pass
            glViewport(0, 0, fbW, fbH);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            lightProg.use();
            setVec3(lightProg.getId(), "uViewPos", cam->cameraPos);
            setInt(lightProg.getId(), "uDebugMode", g_debugMode);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gbuf.gPosition);
            setInt(lightProg.getId(), "gPosition", 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gbuf.gNormal);
            setInt(lightProg.getId(), "gNormal", 1);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gbuf.gAlbedoSpec);
            setInt(lightProg.getId(), "gAlbedoSpec", 2);
            lights.upload(lightProg.getId(), "lights", "numLights");
            glBindVertexArray(fsVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
            glfwSwapBuffers(hWindow);
        }
        destroyGBuffer(gbuf);
        for (auto& kv : texCache) {
            if (kv.second) glDeleteTextures(1, &kv.second);
        }
        if (fsVao) glDeleteVertexArrays(1, &fsVao);
        if (ebo) glDeleteBuffers(1, &ebo);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);

        delete cam;
        cam = nullptr;

        glfwDestroyWindow(hWindow);
        glfwTerminate();
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
