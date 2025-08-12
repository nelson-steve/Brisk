#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform MVPBuffer {
    mat4 projView;
    vec3 CamPos;
} MVP;

layout(push_constant) uniform MeshDataBuffer {
    mat4 model;
    int materialIndex;
} MeshData;

void main() {
    gl_Position = MVP.projView * MeshData.model * vec4(inPosition, 1.0);
}
