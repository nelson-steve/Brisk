#version 450

layout(location = 0) in vec3  inPosition;
layout(location = 1) in vec3  inNormal;
layout(location = 2) in vec2  inUV0;
layout(location = 3) in vec2  inUV1;
layout(location = 4) in vec3  inColor;
layout(location = 5) in vec4  inTangent;
layout(location = 6) in uvec4 inJointIndices;
layout(location = 7) in vec4  inJointWeights;

layout(std140, set = 0, binding = 0) uniform MVPBuffer {
    mat4 ProjView;
    mat4 View;
    vec3 CamPos;
} MVP;

layout(push_constant) uniform MeshDataBuffer {
    mat4 model;
    int materialIndex;
} MeshData;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main() {
    fragPosition = vec3(MeshData.model * vec4(inPosition, 1.0));
    fragNormal = normalize(mat3(transpose(inverse(MeshData.model))) * inNormal);
    fragUV = inUV0;

    gl_Position = MVP.ProjView * MeshData.model * vec4(inPosition, 1.0);
}
