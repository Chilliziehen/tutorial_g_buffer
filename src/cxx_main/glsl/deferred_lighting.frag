#version 430 core

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 uViewPos;

struct Light {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

uniform int numLights;
uniform Light lights[16];

// 0=lit, 1=pos, 2=normal, 3=albedo
uniform int uDebugMode;

void main() {
    vec2 uv = clamp(vUV, vec2(0.0), vec2(1.0));

    vec3 FragPos = texture(gPosition, uv).xyz;
    vec3 Normal  = normalize(texture(gNormal, uv).xyz);
    vec4 AlbedoSpec = texture(gAlbedoSpec, uv);
    vec3 Albedo = AlbedoSpec.rgb;
    float SpecularStrength = AlbedoSpec.a;

    if (uDebugMode == 1) {
        // visualize position (compressed to 0..1 range)
        FragColor = vec4(FragPos * 0.1 + 0.5, 1.0);
        return;
    }
    if (uDebugMode == 2) {
        FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
        return;
    }
    if (uDebugMode == 3) {
        FragColor = vec4(Albedo, 1.0);
        return;
    }

    vec3 lighting = 0.05 * Albedo; // ambient

    vec3 V = normalize(uViewPos - FragPos);

    for (int i = 0; i < numLights; ++i) {
        vec3 Lvec = lights[i].position - FragPos;
        float dist = length(Lvec);
        vec3 L = Lvec / max(dist, 1e-6);

        float attenuation = 1.0 / (lights[i].constant + lights[i].linear * dist + lights[i].quadratic * dist * dist);

        float NdotL = max(dot(Normal, L), 0.0);
        vec3 diffuse = NdotL * Albedo * lights[i].color;

        vec3 H = normalize(L + V);
        float spec = pow(max(dot(Normal, H), 0.0), 32.0);
        vec3 specular = spec * SpecularStrength * lights[i].color;

        lighting += (diffuse + specular) * attenuation;
    }

    FragColor = vec4(lighting, 1.0);
}
