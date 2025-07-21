#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform MVP {
    mat4 projView;
    vec3 CamPos;
} u_MVP;

layout(push_constant) uniform MeshData {
    mat4 model;
    int materialIndex;
} pc_MeshData;

void main() {
    gl_Position = u_MVP.lightSpaceMatrix * pc_MeshData.model * vec4(inPosition, 1.0);
}
