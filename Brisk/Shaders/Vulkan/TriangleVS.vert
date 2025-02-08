#version 450

layout(set = 0, binding = 0) uniform MVPBuffer {
    mat4 Model;
    mat4 View;
    mat4 Projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV0;
layout(location = 3) in vec2 inUV1;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(location = 1) out vec2 UV0;
layout(location = 2) out vec2 UV1;

void main() {
    UV0 = inUV0;
    UV1 = inUV1;
    gl_Position = ubo.Projection * ubo.View * ubo.Model * vec4(inPosition, 1.0f);
    fragColor = inPosition;
}