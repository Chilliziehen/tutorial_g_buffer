#version 430 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec2 gUV;
layout(location = 3) out int  gMaterialIndex;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

// These are per-draw (per-submesh) values.
uniform int uMaterialIndex;

void main() {
    gPosition = vec4(fs_in.FragPos, 1.0);
    gNormal   = normalize(fs_in.Normal);
    gUV = fs_in.TexCoord;
    gMaterialIndex = uMaterialIndex;
    // Pack 4 indices; the extra channel is reserved for future use.
}
