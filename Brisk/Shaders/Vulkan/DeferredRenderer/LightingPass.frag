#version 450

#define MAX_LIGHTS 1024
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS_X 16
#define NUM_CLUSTERS_Y 9
#define NUM_CLUSTERS_Z 24
#define NUM_TOTAL_CLUSTERS (NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z)

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

// G-Buffer textures
layout(set = 2, binding = 0) uniform sampler2D sampler_Position;
layout(set = 2, binding = 1) uniform sampler2D sampler_Normal;
layout(set = 2, binding = 2) uniform sampler2D sampler_Albedo;
layout(set = 2, binding = 3) uniform sampler2D sampler_Material; // metallic roughness occlusion
layout(set = 2, binding = 4) uniform sampler2D sampler_Emissive;
layout(set = 2, binding = 5) uniform sampler2D sampler_Depth;

struct LightData {
    vec4 position; // xyz = pos, w = radius
    vec4 color;    // xyz = color, w = intensity
};

layout(std430, set = 1, binding = 0) readonly buffer LightsList {
    LightData lights[];
} ssbo_LightsList;

layout(std430, set = 5, binding = 2) readonly buffer LightIndices {
    uint lightIndexList[];  // Global array of all light indices
} ssbo_LightIndices;

layout(push_constant) uniform ClusterInfo {
    vec2 ScreenSize;
    float NearZ;
    float FarZ;
} pc_ClusterInfo;

layout(std430, set = 5,  binding = 3) buffer ClusterLightOffsetList {
    uvec2 lightOffsets[]; // start, count per cluster
} ssbo_ClusterLightOffsetList;

uvec3 GetClusterIndex(vec2 screenSize, float nearZ, float farZ, vec2 fragCoord, float linearDepth)
{
    uint clusterX = uint(fragCoord.x / screenSize.x * NUM_CLUSTERS_X);
    uint clusterY = uint(fragCoord.y / screenSize.y * NUM_CLUSTERS_Y);
    
    // normalize depth and assign z cluster
    float zNorm = (linearDepth - nearZ) / (farZ - nearZ);
    uint clusterZ = uint(zNorm * NUM_CLUSTERS_Z);

    return uvec3(clusterX, clusterY, clusterZ);
}

vec3 applyLight(vec3 fragPos, vec3 normal, uint lightIdx)
{
    vec3 lightPos = ssbo_LightsList.lights[lightIdx].position.xyz;
    float radius  = ssbo_LightsList.lights[lightIdx].position.w;
    vec3 lightCol = ssbo_LightsList.lights[lightIdx].color.rgb;
    float intensity = ssbo_LightsList.lights[lightIdx].color.w;

    vec3 toLight = lightPos - fragPos;
    float dist = length(toLight);
    float attenuation = clamp(1.0 - (dist / radius), 0.0, 1.0);
    vec3 lightDir = normalize(toLight);

    float NdotL = max(dot(normal, lightDir), 0.0);
    return lightCol * intensity * attenuation * NdotL;
}

// Constants
const float PI = 3.14159265359;

// --- Helper Functions ---

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// --- Main ---

void main() {
    vec3 pos      = texture(sampler_Position, uv).rgb;
    vec3 N        = normalize(texture(sampler_Normal, uv).rgb);
    vec3 albedo   = texture(sampler_Albedo, uv).rgb;
    float alpha   = texture(sampler_Albedo, uv).a;
    vec3 emissive = texture(sampler_Emissive, uv).rgb;
    vec4 matData  = texture(sampler_Material, uv);
    float depth   = texture(sampler_Depth, uv).r;

    float linearDepth = (2.0 * pc_ClusterInfo.NearZ) / (pc_ClusterInfo.FarZ + pc_ClusterInfo.NearZ - depth * (pc_ClusterInfo.FarZ - pc_ClusterInfo.NearZ));

    uvec3 cluster = GetClusterIndex(pc_ClusterInfo.ScreenSize, pc_ClusterInfo.NearZ, pc_ClusterInfo.FarZ, gl_FragCoord.xy, linearDepth);
    uint clusterIndex = cluster.x + cluster.y * NUM_CLUSTERS_X + cluster.z * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;

    outColor = vec4(albedo, alpha);
}
