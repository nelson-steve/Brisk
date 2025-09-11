#version 450

layout(location = 0) in vec3 inPosition;

layout(std140, set = 0, binding = 0) uniform MVPBuffer {
    mat4 ProjView;
    mat4 View;
    vec3 CamPos;
} MVP;

layout(push_constant) uniform MeshDataBuffer {
    mat4 model;
} MeshData;

void main() {
    gl_Position = MVP.ProjView * MeshData.model * vec4(inPosition, 1.0);
}
