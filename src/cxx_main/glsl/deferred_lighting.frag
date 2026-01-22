#version 430 core

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D gPosition;        //texture 0
uniform sampler2D gNormal;          //texture 1
uniform sampler2D gUV;              //texture 2
uniform isampler2D gMaterialIndex;  //texture 3
uniform isampler2D gTexIndices0;    //texture 4

uniform vec3 uViewPos;

struct Material{
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    int doubleSided;
    int albedoTexture;
    int normalTexture;
    int metallicRoughnessTexture;
    int aoTexture;
    int emissiveTexture;
};

//uniform int numLights;
//uniform Light lights[16];

uniform int uMaterialCount;
uniform Material uMaterials[32];

uniform int uTextureCount;
uniform sampler2D uTextures[32];

struct PointLight{
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight uPointLight;

void main() {
    vec3 FragPos = texture(gPosition, vUV).xyz;
    vec3 Normal = normalize(texture(gNormal, vUV).xyz);
    vec2 uv = texture(gUV, vUV).xy;
    int materialIndex = texture(gMaterialIndex, vUV).r;
    if(FragPos.x == 0.0f && FragPos.y == 0.0f && FragPos.z == 0.0f){
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    Material material = uMaterials[materialIndex];
    if(material.albedoTexture>=0){
        int albedoTexIndex = material.albedoTexture;
        vec4 albedoTex = texture(uTextures[albedoTexIndex], uv);
        FragColor = albedoTex;
        return;
    }
    if (FragPos.x ==0.0f && FragPos.y ==0.0f && FragPos.z ==0.0f) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    //Material material = uMaterials[materialIndex];
    FragColor = vec4(vUV,0.0f,1.0f);
    //FragColor = texture(material.albedoTexture, uv);
}
