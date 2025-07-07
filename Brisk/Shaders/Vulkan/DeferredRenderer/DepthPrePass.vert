#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform UBO {
    mat4 projView;
    vec3 CamPos;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    int materialIndex;
} pc;

void main() {
    gl_Position = ubo.projView * pc.model * vec4(inPosition, 1.0);
}
