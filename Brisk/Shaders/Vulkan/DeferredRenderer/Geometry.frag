#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

layout(set = 1, binding = 0) uniform SceneLights {
    vec3 lightPos;
    vec3 lightColor;
    vec3 viewPos;
} lights;

layout(set = 2, binding = 0) uniform sampler2D albedoTexture;

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 3, binding = 0) uniform sampler2D global_textures[];      // 2D textures
layout(set = 3, binding = 0) uniform sampler3D global_textures_3d[];   // 3D textures

void main() {
    outPosition = vec4(fragPosition, 1.0);
    outNormal = vec4(normalize(fragNormal), 1.0);
    outAlbedo = texture(albedoTexture, fragUV);
}
