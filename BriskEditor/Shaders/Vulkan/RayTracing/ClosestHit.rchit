#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"

layout(location = 0) rayPayloadInEXT RadiancePayload radiancePayload;
layout(location = 2) rayPayloadEXT ShadowPayload shadowPayload;

struct Vertex {
    float vx, vy, vz;
    float nx, ny, nz;
    float tx, ty;
};

struct Mesh {
    vec3 center;
    float radius;

    uint vertexOffset;
    uint vertexCount;
    uint meshletCount;
    uint materialIndex;

    uint meshletOffset;
    uint indexCount;   
    uint indexOffset;  
    uint _pad0;
};


struct MeshDraw
{
    uint transformIndex;
    uint meshIndex;
    uint materialIndex;
    uint meshletCount;
    uint meshletOffset;

    uint groupcount[3];
};

struct MaterialData {
    uint alphaMode; // use 4 bytes to avoid packing issues
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;

    vec4 baseColorFactor;
    vec3 emissiveFactor;

    float emissiveStrength;

    uint baseColorTextureIndex;
    uint metallicRoughnessTextureIndex;
    uint normalTextureIndex;
    uint occlusionTextureIndex;
    uint emissiveTextureIndex;
};

layout(std430, set = 0, binding = 3) readonly buffer VerticesBuffer {
    Vertex vertices[];
} Vertices;

layout(std430, set = 0, binding = 14) readonly buffer IndicesBuffer {
    uint indices[];
} Indices;

layout(std430, set = 0, binding = 15) readonly buffer MeshesBuffer {
    Mesh meshes[];
} Meshes;

layout(set = 1, binding = 0) uniform sampler2D GlobalTextures[];

layout(set = 0, binding = 11) uniform accelerationStructureEXT topLevelAS;

layout(std430, set = 0, binding = 2) readonly buffer MaterialsBuffer {
    MaterialData materials[];
} Materials;

layout(set = 0, binding = 9) readonly buffer MeshDrawsBuffer {
	MeshDraw meshDraws[];
} MeshDraws;

layout(set = 0, binding = 13) uniform CameraUBO {
    mat4 viewInv;
    mat4 projInv;
    vec3 lightPos;
    vec3 camPos;
    vec2 dimension; // width, height
    vec2 _pad;
} camera;

hitAttributeEXT vec2 attribs;

void main() {
    MeshDraw draw = MeshDraws.meshDraws[gl_InstanceID];
    MaterialData material = Materials.materials[draw.materialIndex];

    Mesh mesh = Meshes.meshes[draw.meshIndex];

    uint vertexOffset = mesh.vertexOffset;
    uint indexOffset = mesh.indexOffset;

    uint triIndex = gl_PrimitiveID;

    uint i0 = Indices.indices[indexOffset + triIndex * 3 + 0];
    uint i1 = Indices.indices[indexOffset + triIndex * 3 + 1];
    uint i2 = Indices.indices[indexOffset + triIndex * 3 + 2];

    vec2 uv0 = vec2(Vertices.vertices[vertexOffset + i0].tx, Vertices.vertices[vertexOffset + i0].ty);
    vec2 uv1 = vec2(Vertices.vertices[vertexOffset + i1].tx, Vertices.vertices[vertexOffset + i1].ty);
    vec2 uv2 = vec2(Vertices.vertices[vertexOffset + i2].tx, Vertices.vertices[vertexOffset + i2].ty);

    float alpha = 1.0 - attribs.x - attribs.y;
    float beta  = attribs.x;
    float gamma = attribs.y;

    vec2 uv = alpha * uv0 + beta * uv1 + gamma * uv2;

    vec4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureIndex != 0) {
        baseColor *= texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], uv);
    }

    radiancePayload.color = baseColor.xyz;

	vec3 lightVector = normalize(-camera.lightPos);
    vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	float tmin = 0.001;
	float tmax = 10000.0;

    shadowPayload.occluded = 1u;

    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
    if(shadowPayload.occluded == 1u){
        radiancePayload.color *= vec3(0.2);
    }
}