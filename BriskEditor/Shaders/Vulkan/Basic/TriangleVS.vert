#version 450

layout(set = 0, binding = 0) uniform MVPBuffer {
    mat4 Model;
    mat4 View;
    mat4 Projection;
    vec3 CamPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec2 UV0;
layout(location = 1) out vec2 UV1;
layout(location = 2) out vec3 WorldPos;
layout(location = 3) out vec3 Normal;
layout(location = 4) out vec3 CamPos;

void main() {
    UV0 = inUV0;
    UV1 = inUV1;
    CamPos = ubo.CamPos;
    WorldPos = vec3(ubo.Model * vec4(inPosition, 1.0));
    Normal = mat3(ubo.Model) * inNormal;
    gl_Position = ubo.Projection * ubo.View * ubo.Model * vec4(inPosition, 1.0f);
}