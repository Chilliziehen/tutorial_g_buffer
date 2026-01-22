//
// Created by 30367 on 2025/10/28.
//
#include <fstream>
#include "Utilities.h"
#include <fstream>
#include <iostream>
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <GL/glew.h>
#include <gtc/type_ptr.hpp>

namespace Utilities {
    const char* rdFile(const char* filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("file not found: " + std::string(filePath));
        }

        // 获取文件大小
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 分配缓冲区
        char* buf = new char[size + 1];
        if (!file.read(buf, size)) {
            delete[] buf;
            throw std::runtime_error("failed to read file: " + std::string(filePath));
        }

        buf[size] = '\0';
        file.close();
        return buf;
    }

    uint32_t createTexture2DFromFile(const std::string& path) {
        int w = 0, h = 0, n = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);
        if (!data) {
            throw std::runtime_error("Failed to load texture: " + path);
        }

        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;
        if (n == 1) { format = GL_RED; internalFormat = GL_R8; }
        else if (n == 3) { format = GL_RGB; internalFormat = GL_SRGB8; }
        else if (n == 4) { format = GL_RGBA; internalFormat = GL_SRGB8_ALPHA8; }

        uint32_t tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        return tex;
    }
    void drainGlErrors(const char* tag) {
        for (GLenum e = glGetError(); e != GL_NO_ERROR; e = glGetError()) {
            std::cerr << "[GL_ERROR] " << tag << " : 0x" << std::hex << e << std::dec << std::endl;
        }
    }
    void calculateGlobalTransforms(scene::RenderScene *rs) {
        typedef std::pair<scene::Node*, glm::mat4> NodeTransformPair;
        auto genTransform = [](const scene::Transform& t){
            return glm::translate(glm::mat4(1.0f), t.position) *
                   glm::mat4_cast(t.rotation) *
                   glm::scale(glm::mat4(1.0f), t.scale);
        };
        glm::mat4 rootTransform = genTransform(rs->nodes[rs->rootIndex].localTransform);
        std::vector<NodeTransformPair> stack;
        stack.emplace_back(&rs->nodes[rs->rootIndex], rootTransform);
        while (!stack.empty()) {
            NodeTransformPair current = stack.back();
            stack.pop_back();
            scene::Node* node = current.first;
            glm::mat4 globalTransform = current.second;
            node->globalTransform = globalTransform;
            for (int childIndex : node->children) {
                if (childIndex >= 0 && static_cast<size_t>(childIndex) < rs->nodes.size()) {
                    scene::Node* childNode = &rs->nodes[childIndex];
                    glm::mat4 childLocalTransform = genTransform(childNode->localTransform);
                    glm::mat4 childGlobalTransform = globalTransform * childLocalTransform;
                    stack.emplace_back(childNode, childGlobalTransform);
                }
            }
        }
    }

}
