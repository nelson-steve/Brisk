#version 460
#extension GL_EXT_shader_image_load_formatted : require

layout(location = 0) in vec3 gWorldPos;
layout(location = 1) in vec3 gNormal;
layout(location = 2) in vec2 gUV;

layout(set = 0, binding = 5, rgba8) uniform image3D voxelGrid;

// layout(set = 0, binding = 2) uniform VoxelData {
//     vec3 gridMin;
//     vec3 gridMax;
//     ivec3 resolution;
// } voxelInfo;

void main() {
    vec3 gridMin = vec3(-100.0, -100.0, -100.0);
    vec3 gridMax = vec3(100.0, 100.0, 100.0);
    ivec3 resolution = ivec3(256, 256, 256);

    vec3 rel = (gWorldPos - gridMin) / (gridMax - gridMin);
    ivec3 voxelCoord = ivec3(rel * vec3(resolution));

    if (any(lessThan(voxelCoord, ivec3(0))) || any(greaterThanEqual(voxelCoord, resolution)))
        return;

    vec4 color = vec4(abs(gNormal), 1.0);
    imageStore(voxelGrid, voxelCoord, color);
}
