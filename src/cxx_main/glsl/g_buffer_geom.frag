#version 430 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform sampler2D uAlbedoMap;
uniform bool uHasAlbedoMap;
uniform vec4 uBaseColorFactor;

// Demo: pack a simple specular strength into alpha
uniform float uSpecular;

void main() {
    gPosition = vec4(fs_in.FragPos, 1.0);
    gNormal   = vec4(normalize(fs_in.Normal), 1.0);

    vec3 albedo = uBaseColorFactor.rgb;
    if (uHasAlbedoMap) {
        albedo *= texture(uAlbedoMap, fs_in.TexCoord).rgb;
    }

    gAlbedoSpec = vec4(albedo, uSpecular);
}

