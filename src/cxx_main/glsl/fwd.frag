#version 430
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

uniform Light lights[8];
uniform int numLights;

struct Material{
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    int doubleSided;
    sampler2D albedoTexture;
    sampler2D normalTexture;
    sampler2D metallicRoughnessTexture;
    sampler2D aoTexture;
    sampler2D emissiveTexture;
};

uniform Material matrial;

void main() {
    vec3 normallized = fs_in.FragPos+vec3(1.0f,1.0f,1.0f);
    normallized = normallized / 2.0f;
    FragColor = vec4(normallized, 1.0);
}