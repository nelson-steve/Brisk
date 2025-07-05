#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 0, binding = 0) uniform UBO {
    mat4 projView;
    vec3 CamPos;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    int materialIndex;
} pc;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main() {
    fragPosition = vec3(pc.model * vec4(inPosition, 1.0));
    fragNormal = mat3(transpose(inverse(pc.model))) * inNormal;
    fragUV = inUV;

    gl_Position = ubo.projView * pc.model * vec4(inPosition, 1.0);
}
