#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform MeshDataBuffer {
    mat4 matrix;
} MeshData;

void main() {
    gl_Position = MeshData.matrix * mat4(1.0) * vec4(inPosition, 1.0);
}
