#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Tangent;
    vec3 Bitangent;
} fs_in;


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

uniform Material uMaterial;

// 前向渲染中，我们将纹理采样器绑定到固定槽位：
// 0: Albedo, 1: Normal, 2: Specular/Metal, 3: AO, 4: Emissive
uniform sampler2D uTexAlbedo;    // slot 0
uniform sampler2D uTexNormal;    // slot 1
uniform sampler2D uTexSpec;      // slot 2
uniform sampler2D uTexAO;        // slot 3
uniform sampler2D uTexEmissive;  // slot 4

// --- 灯光结构 ---
struct pointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};
uniform pointLight uPointLight;
uniform vec3 uViewPos;

void main() {
    // 基础颜色 
    vec4 albedoRGBA = texture(uTexAlbedo, fs_in.TexCoord); // 先读出来
    if (albedoRGBA.a < 0.1) discard; // 透明度测试
    
    // 伽马矫正 SRGB -> Linear
    vec3 albedo = pow(albedoRGBA.rgb, vec3(2.2)); 


    // 法线
    vec3 N;
    if (uMaterial.normalTexture >= 0) {
        vec3 normalMapValue = texture(uTexNormal, fs_in.TexCoord).rgb;
        normalMapValue = normalize(normalMapValue * 2.0 - 1.0);
        vec3 T = normalize(fs_in.Tangent);
        vec3 B = normalize(fs_in.Bitangent);
        vec3 geometricN = normalize(fs_in.Normal);
        mat3 TBN = mat3(T, B, geometricN);
        N = normalize(TBN * normalMapValue);
    } else {
        N = normalize(fs_in.Normal);
    }

    vec3 V = normalize(uViewPos - fs_in.FragPos);
    if (uMaterial.doubleSided == 1 && dot(N, V) < 0.0) N = -N;

    vec3 specColorFromMap = vec3(0.04); // 默认绝缘体高光
    float glossiness = 0.5;             // 默认中等光滑
    
    if (uMaterial.metallicRoughnessTexture >= 0) {
        vec4 specSample = texture(uTexSpec, fs_in.TexCoord);
        specColorFromMap = specSample.rgb; // 读取 RGB 作为高光颜色
        glossiness = specSample.a;         // 读取 Alpha 作为光泽度
    }

    // AO 读取
    float ao = 1.0;
    if (uMaterial.aoTexture >= 0) {
        ao = texture(uTexAO, fs_in.TexCoord).r;
    }

    // 光照计算
    vec3 Lvec = uPointLight.position - fs_in.FragPos;
    float dist = length(Lvec);
    vec3 L = normalize(Lvec);
    vec3 H = normalize(L + V);
    float attenuation = 1.0 / (uPointLight.constant + uPointLight.linear * dist + uPointLight.quadratic * dist * dist);

    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * albedo * uPointLight.color;

    // Specular (Blinn-Phong)
    // Glossiness (0~1) 映射到 Shininess (2~256)
    float shininess = exp2(glossiness * 8.0 + 1.0); // 经验公式，让高光变化更自然
    float specFactor = pow(max(dot(N, H), 0.0), shininess);
    
    // 高光颜色 = 贴图颜色 * 强度 * 灯光颜色
    vec3 specular = specFactor * specColorFromMap * uPointLight.color; 

    // Ambient (稍微调亮一点环境光，防止死黑)
    vec3 ambient = 0.38 * albedo * ao; // 这里不用灯光颜色乘了，模拟全局白光

    // 组合
    vec3 finalColor = (ambient + (diffuse + specular) * attenuation);

    // 伽马矫正 (Linear -> sRGB)
    finalColor = pow(finalColor, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}