#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// layout(push_constant) uniform MeshDataBuffer {
//     mat4 model;
//     int materialIndex;
// } MeshData;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

void main() {
    mat4 model = mat4(1.0);
    vec4 worldPos = model * vec4(inPosition, 1.0);
    outWorldPosition = worldPos.xyz;
    outNormal = normalize(mat3(model) * inNormal);
    outUV = inUV;

    gl_Position = worldPos;
}