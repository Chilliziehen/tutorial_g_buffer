/*
* filename：LightManager.h
 * arthur：Chilliziehen
 * time created：2026/1/19
 * description：
 */
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <GL/glew.h>
#include "Light.h"

class LightManager {
public:
    static constexpr int MAX_LIGHTS = 16;

    void add(const Light& l) { lights.push_back(l); }
    void clear() { lights.clear(); }
    size_t size() const { return lights.size(); }
    const std::vector<Light>& get() const { return lights; }
    std::vector<Light>& get() { return lights; }

    // 上传 numLights 和 lights[i] 到指定 Program
    void upload(GLuint programId,
                const std::string& uniformArrayName = "lights",
                const std::string& numUniformName = "numLights") const;

private:
    std::vector<Light> lights;
};

