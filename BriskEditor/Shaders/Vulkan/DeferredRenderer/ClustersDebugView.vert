#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants {
    mat4 proj;
} PC;

void main() {
    gl_Position = proj * vec4(inPosition, 1.0);
}
