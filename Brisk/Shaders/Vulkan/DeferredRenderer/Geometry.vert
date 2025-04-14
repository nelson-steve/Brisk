#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main() {
    mat4 modelView = ubo.view * ubo.model;
    fragPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragUV = inUV;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}
