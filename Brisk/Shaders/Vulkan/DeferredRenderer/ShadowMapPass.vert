#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform UBO {
    mat4 lightSpaceMatrix; // Light's view-projection matrix
    mat4 model;            // Model matrix of the object
} ubo;

void main() {
    gl_Position = ubo.lightSpaceMatrix * ubo.model * vec4(inPosition, 1.0);
}
