#version 430 core

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D  gPosition;        // texture 0
uniform sampler2D  gNormal;          // texture 1
uniform sampler2D  gUV;              // texture 2
uniform isampler2D gMaterialIndex;   // texture 3

uniform vec3 uViewPos;

struct Material{
    vec4  baseColorFactor;
    float metallicFactor;    // Phong里不用也没事
    float roughnessFactor;   // 用来推一个 shininess
    int   doubleSided;
    int   albedoTexture;     // index into uTextures, -1 means none
    int   normalTexture;
    int   metallicRoughnessTexture;
    int   aoTexture;
    int   emissiveTexture;
};

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

    // 你现在用 (0,0,0) 当“没有几何”的标记；更稳的是写一个 gDepth 或 gMask
    if (FragPos == vec3(0.0)) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 N = texture(gNormal, vUV).xyz;
    float nlen = length(N);
    if (nlen < 1e-5) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    N = normalize(N);

    vec2 uv = texture(gUV, vUV).xy;

    int materialIndex = texture(gMaterialIndex, vUV).r;
    if (materialIndex < 0) {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0); // 紫色：材质索引错误
        return;
    }

    Material material = uMaterials[materialIndex];

    // 双面材质：让法线朝向视线一侧（deferred里常用）
    if (material.doubleSided != 0) {
        vec3 Vtmp = normalize(uViewPos - FragPos);
        if (dot(N, Vtmp) < 0.0) N = -N;
    }

    // --- Albedo ---
    vec3 albedo = material.baseColorFactor.rgb;
    if (material.albedoTexture >= 0) {
        albedo *= texture(uTextures[material.albedoTexture], uv).rgb;
    }

    // --- Lighting vectors ---
    vec3 Lvec = uPointLight.position - FragPos;
    float dist = length(Lvec);
    vec3 L = (dist > 1e-5) ? (Lvec / dist) : vec3(0.0);

    vec3 V = normalize(uViewPos - FragPos);
    vec3 H = normalize(L + V); // Blinn-Phong half vector

    // --- Attenuation (point light) ---
    float attenuation = 1.0 / (uPointLight.constant +
    uPointLight.linear * dist +
    uPointLight.quadratic * dist * dist);

    // --- Ambient / Diffuse / Specular ---
    float NdotL = max(dot(N, L), 0.0);

    // roughness -> shininess 映射（你也可以直接给一个 uniform specPower）
    float r = clamp(material.roughnessFactor, 0.0, 1.0);
    float shininess = mix(256.0, 8.0, r); // 越粗糙越“糊”，高光指数越小

    // Blinn-Phong spec
    float spec = 0.0;
    if (NdotL > 0.0) {
        spec = pow(max(dot(N, H), 0.0), shininess);
    }

    vec3 lightColor = uPointLight.color;

    vec3 ambient  = 0.7 * albedo;                         // 环境光强度你可调
    vec3 diffuse  = NdotL * albedo * lightColor;
    vec3 specular = spec * lightColor;                     // 也可以乘一个 specColor

    vec3 color = ambient + attenuation * (diffuse + specular);
    /*
    if(material.normalTexture>=0){
        int albedoTexIndex = material.normalTexture;
        vec4 albedoTex = texture(uTextures[albedoTexIndex], uv);
        FragColor = albedoTex;
        return;
    }
*/
    FragColor = vec4(color, 1.0);
}
