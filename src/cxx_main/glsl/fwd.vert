#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;   // 【新增】
layout (location = 4) in vec3 aBitangent; // 【新增】

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;   // 【传给 Frag】
    vec3 Bitangent; // 【传给 Frag】
} vs_out;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoord = aTexCoords;
    
    // 法线矩阵
    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.Tangent = normalMatrix * aTangent;     // 【计算切线】
    vs_out.Bitangent = normalMatrix * aBitangent; // 【计算副切线】

    gl_Position = uProj * uView * worldPos;
}