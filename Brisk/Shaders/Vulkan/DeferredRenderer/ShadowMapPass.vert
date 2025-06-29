#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 1, binding = 0) uniform UBO {
    mat4 lightSpaceMatrix;
    mat4 model;
} ubo;

void main() {
    gl_Position = ubo.lightSpaceMatrix * ubo.model * vec4(inPosition, 1.0);
}
