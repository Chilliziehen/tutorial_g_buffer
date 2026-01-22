#version 430 core
layout(location = 0) in vec2 quadPosition; // 0..1
out vec2 vUV;

void main() {
    vUV = quadPosition;                         // 给 fragment 用的 UV
    vec2 clip = quadPosition * 2.0 - 1.0;       // 0..1 -> -1..1
    gl_Position = vec4(clip, 0.0, 1.0);
}
