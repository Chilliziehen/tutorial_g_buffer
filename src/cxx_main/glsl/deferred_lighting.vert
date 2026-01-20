#version 430 core

// Fullscreen triangle (no VBO; uses gl_VertexID)
const vec2 positions[3] = vec2[3](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

out vec2 vUV;

void main() {
    vec2 p = positions[int(gl_VertexID)];
    gl_Position = vec4(p, 0.0, 1.0);
    // map from NDC to UV. Note: p can be outside [-1,1] for the oversized triangle.
    vUV = p * 0.5 + 0.5;
}
