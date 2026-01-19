#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"
#include "RTHelpers.glsl"

#define PI 3.14159265359

layout(location = 0) rayPayloadInEXT RadiancePayload radiancePayload;

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
    uint alphaMode;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    bool doubleSided;

    vec4 baseColorFactor;
    vec3 emissiveFactor;

    float emissiveStrength;

    int baseColorTextureIndex;
    int metallicRoughnessTextureIndex;
    int normalTextureIndex;
    int occlusionTextureIndex;
    int emissiveTextureIndex;
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
    uint frame;
    uint pad;
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

vec3 rotateQuat(vec3 v, vec4 q)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

vec3 sampleGGX(vec3 N, float roughness, inout uint rng)
{
    float a  = roughness * roughness;
    float a2 = a * a;

    float r1 = Random(rng);
    float r2 = Random(rng);

    float phi = 2.0 * PI * r1;

    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 - 1.0) * r2));
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    // GGX half-vector in tangent space
    vec3 Ht = vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    // Build TBN basis
    vec3 T = normalize(abs(N.z) < 0.999
        ? cross(N, vec3(0, 0, 1))
        : cross(N, vec3(0, 1, 0)));

    vec3 B = cross(N, T);

    // Transform to world space
    return normalize(T * Ht.x + B * Ht.y + N * Ht.z);
}

vec3 CosineHemisphere(vec3 N, inout uint rng)
{
    float r1 = Random(rng);
    float r2 = Random(rng);

    float phi = 2.0 * PI * r1;

    float cosTheta = sqrt(1.0 - r2);
    float sinTheta = sqrt(r2);

    // Local (tangent-space) direction
    vec3 Lh = vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    // Build orthonormal basis (TBN)
    vec3 T = normalize(abs(N.z) < 0.999
        ? cross(N, vec3(0, 0, 1))
        : cross(N, vec3(0, 1, 0)));

    vec3 B = cross(N, T);

    // Transform to world space
    return normalize(T * Lh.x + B * Lh.y + N * Lh.z);
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
    if (material.normalTextureIndex != -1) {
        NormalMap = texture(GlobalTextures[nonuniformEXT(material.normalTextureIndex)], uv).xyz * 2.0 - 1.0;

        N = normalize(Tangent.xyz * NormalMap.x + B * NormalMap.y + N * NormalMap.z);
    }

    vec4 albedo = material.baseColorFactor;
    if (material.baseColorTextureIndex != -1) {
        albedo = texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], uv);
    }
    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != -1) {
        vec4 sampleTex = texture(GlobalTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], uv);
        roughness = sampleTex.g;
        metallic = sampleTex.b;

        roughness = clamp(roughness, 0.04, 1.0);
        metallic  = clamp(metallic, 0.0, 1.0);
    }
    float occlusion = 1.0;
    if (material.occlusionTextureIndex != -1) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], uv).r;
    }

    vec3 V = normalize(-gl_WorldRayDirectionEXT);

    vec3 P = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    P += N * max(1e-4, 1e-4 * gl_HitTEXT);


    vec3 emission = material.emissiveFactor * material.emissiveStrength;
    if (material.emissiveTextureIndex != -1)
        emission *= texture(GlobalTextures[nonuniformEXT(material.emissiveTextureIndex)], uv).rgb;

    radiancePayload.radiance += radiancePayload.throughput * emission;

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    float specProb = max(F0.r, max(F0.g, F0.b));
    float r = Random(radiancePayload.rngState);

    vec3 newDir;
    vec3 brdf;
    float pdf;

    if (r < specProb) {
        // GGX specular
        vec3 H = sampleGGX(N, roughness, radiancePayload.rngState);
        newDir = reflect(-V, H);

        float NdotL = max(dot(N, newDir), 0.0); 

        if (dot(N, newDir) <= 0.0) {
            radiancePayload.done = true;
            return;
        }

        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        vec3 F = FSchlick(F0, VdotH);
        float D = DGGX(NdotH, roughness);
        float G = GSmith(NdotV, NdotL, roughness);

        brdf = (D * G * F) / max(4.0 * NdotL * NdotV, 1e-6);
        pdf  = D * NdotH / max(4.0 * VdotH, 1e-6);
        //pdf = (D * NdotH) / (4.0 * abs(VdotH));
    } else {
        // Diffuse
        newDir = CosineHemisphere(N, radiancePayload.rngState);
        brdf = albedo.rgb / PI;
        pdf = (max(dot(N, newDir), 0.0) / PI);
    }

    if (pdf <= 0.0) {
        radiancePayload.done = true;
        return;
    }

    radiancePayload.throughput *= brdf * max(dot(N, newDir), 0.0) / pdf;

    float maxThroughput = 10.0;
    radiancePayload.throughput =
        min(radiancePayload.throughput, vec3(maxThroughput));

    radiancePayload.nextOrigin = P;
    radiancePayload.nextDir    = normalize(newDir);
}