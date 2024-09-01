#version 450

layout(binding = 0) uniform MVPBuffer {
    mat4 Model;
    mat4 View;
    mat4 Projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.Projection * ubo.View * ubo.Model * vec4(inPosition, 1.0);
    //gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;
}