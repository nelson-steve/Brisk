#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"

#define PI 3.14159265359

layout(location = 0) rayPayloadInEXT RadiancePayload radiancePayload;
layout(location = 2) rayPayloadEXT ShadowPayload shadowPayload;

struct Vertex {
    float vx, vy, vz;
    float nx, ny, nz;
    float tx, ty;
    float tpx, tpy, tpz, tpw;
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

struct Transform {
    vec3 position;
    float scale;
    vec4 orientation;
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

layout(set = 0, binding = 10) readonly buffer TransformsBuffer {
	Transform transforms[];
} Transforms;

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

// Trowbridge-Reitz / GGX
float DGGX(float NdotH, float roughness) {
    float a2 = roughness * roughness;
    float a2_ = a2 * a2;
    float denom = (NdotH * NdotH) * (a2_ - 1.0) + 1.0;
    return a2_ / (PI * denom * denom);
}

// Schlick-GGX (Smith)
float GSchlickGGX(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GSmith(float NdotV, float NdotL, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return GSchlickGGX(NdotV, k) * GSchlickGGX(NdotL, k);
}

// Schlick approximation
vec3 FSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CookTorranceSpecular(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float D = DGGX(NdotH, roughness);
    float G = GSmith(NdotV, NdotL, roughness);
    vec3 F = FSchlick(F0, VdotH);

    vec3 numerator = D * G * F;
    float denom = 4.0 * NdotV * NdotL + 1e-5;
    return numerator / denom;
}

vec3 DiffuseLambert(vec3 albedo) {
    return albedo / PI;
}

vec3 evaluateLight(
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 radiance,
    float ao
) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Fresnel F0
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance
    float D = DGGX(NdotH, roughness);
    float G = GSmith(NdotV, NdotL, roughness);
    vec3 F = FSchlick(F0, VdotH);

    vec3 specNumer = D * G * F;
    float denom = max(4.0 * NdotV * NdotL, 1e-6);
    vec3 spec = specNumer / denom;

    // Energy-conserving diffuse
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * (albedo / PI);

    // final contribution from this light
    return (diffuse + spec) * radiance * NdotL;
}

vec3 rotateQuat(vec3 v, vec4 q)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main() {
    MeshDraw draw = MeshDraws.meshDraws[gl_InstanceID];
    MaterialData material = Materials.materials[draw.materialIndex];

    Mesh mesh = Meshes.meshes[draw.meshIndex];
    Transform transform = Transforms.transforms[draw.transformIndex];

    uint vertexOffset = mesh.vertexOffset;
    uint indexOffset = mesh.indexOffset;

    uint triIndex = gl_PrimitiveID;

    float alpha = 1.0 - attribs.x - attribs.y;
    float beta  = attribs.x;
    float gamma = attribs.y;

    uint i0 = Indices.indices[indexOffset + triIndex * 3 + 0];
    uint i1 = Indices.indices[indexOffset + triIndex * 3 + 1];
    uint i2 = Indices.indices[indexOffset + triIndex * 3 + 2];

    vec2 uv0 = vec2(Vertices.vertices[vertexOffset + i0].tx, Vertices.vertices[vertexOffset + i0].ty);
    vec2 uv1 = vec2(Vertices.vertices[vertexOffset + i1].tx, Vertices.vertices[vertexOffset + i1].ty);
    vec2 uv2 = vec2(Vertices.vertices[vertexOffset + i2].tx, Vertices.vertices[vertexOffset + i2].ty);

    vec3 N0 = vec3(
        Vertices.vertices[vertexOffset + i0].nx,
        Vertices.vertices[vertexOffset + i0].ny,
        Vertices.vertices[vertexOffset + i0].nz
    );

    vec3 N1 = vec3(
        Vertices.vertices[vertexOffset + i1].nx,
        Vertices.vertices[vertexOffset + i1].ny,
        Vertices.vertices[vertexOffset + i1].nz
    );

    vec3 N2 = vec3(
        Vertices.vertices[vertexOffset + i2].nx,
        Vertices.vertices[vertexOffset + i2].ny,
        Vertices.vertices[vertexOffset + i2].nz
    );

    vec3 N = normalize(
        alpha * N0 +
        beta  * N1 +
        gamma * N2
    );

    vec4 T0 = vec4(
        Vertices.vertices[vertexOffset + i0].tpx,
        Vertices.vertices[vertexOffset + i0].tpy,
        Vertices.vertices[vertexOffset + i0].tpz,
        Vertices.vertices[vertexOffset + i0].tpw
    );

    vec4 T1 = vec4(
        Vertices.vertices[vertexOffset + i1].tpx,
        Vertices.vertices[vertexOffset + i1].tpy,
        Vertices.vertices[vertexOffset + i1].tpz,
        Vertices.vertices[vertexOffset + i1].tpw
    );

    vec4 T2 = vec4(
        Vertices.vertices[vertexOffset + i2].tpx,
        Vertices.vertices[vertexOffset + i2].tpy,
        Vertices.vertices[vertexOffset + i2].tpz,
        Vertices.vertices[vertexOffset + i2].tpw
    );

    vec4 Tangent = normalize(
        alpha * T0 +
        beta  * T1 +
        gamma * T2
    );

    vec2 uv = alpha * uv0 + beta * uv1 + gamma * uv2;

    Tangent = vec4(rotateQuat(Tangent.xyz, transform.orientation), Tangent.w);
    N = rotateQuat(N, transform.orientation);

    // Orthonormalize
    Tangent = vec4(normalize(Tangent.xyz - N * dot(N, Tangent.xyz)), Tangent.w);
    vec3 B = cross(N, Tangent.xyz) * Tangent.w;

    vec3 NormalMap;
    if (material.normalTextureIndex != 0) {
        NormalMap = texture(GlobalTextures[nonuniformEXT(material.normalTextureIndex)], uv).xyz * 2.0 - 1.0;

        N = normalize(Tangent.xyz * NormalMap.x + B * NormalMap.y + N * NormalMap.z);
    }

    vec4 albedo = material.baseColorFactor;
    if (material.baseColorTextureIndex != 0) {
        albedo = texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], uv);
    }
    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != 0) {
        vec4 sampleTex = texture(GlobalTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], uv);
        roughness = sampleTex.g;
        metallic = sampleTex.b;

        roughness = clamp(roughness, 0.04, 1.0);
        metallic  = clamp(metallic, 0.0, 1.0);
    }
    float occlusion = 1.0;
    if (material.occlusionTextureIndex != 0) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], uv).r;
    }

    vec3 V = normalize(gl_WorldRayDirectionEXT);
	vec3 lightVector = normalize(-camera.lightPos);

    vec3 P = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3 L = normalize(camera.lightPos - P);

    vec3 accum = vec3(0.0);
    vec3 sunColor = vec3(1.0, 0.95, 0.9);
    float sunIntensity = 6.0;
    vec3 radiance = sunColor * sunIntensity;
    accum +=  evaluateLight(albedo.rgb, metallic, roughness, NormalMap, V, lightVector, radiance, occlusion);

    vec3 ambient = vec3(0.03);
    ambient = ambient * albedo.rgb * occlusion;
    vec3 finalColor = ambient + accum;

    finalColor = finalColor / (finalColor + vec3(1.0));

    radiancePayload.radiance = finalColor;

    vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	float tmin = 0.001;
	float tmax = 10000.0;

    shadowPayload.occluded = 1u;

    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
    if(shadowPayload.occluded == 1u){
        radiancePayload.radiance *= vec3(0.2);
    }
}