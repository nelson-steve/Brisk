#version 450

#define PI 3.14159265359

#include "../config.hpp"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform MVPBuffer {
    mat4 ProjView;
    mat4 View;
    vec3 CamPos;
} MVP;

// G-buffer textures
layout(set = 2, binding = 0) uniform sampler2D sampler_Position;
layout(set = 2, binding = 1) uniform sampler2D sampler_Normal;
layout(set = 2, binding = 2) uniform sampler2D sampler_Albedo;
layout(set = 2, binding = 3) uniform sampler2D sampler_Material; // metal/rough/occlusion
layout(set = 2, binding = 4) uniform sampler2D sampler_Emissive;
layout(set = 2, binding = 5) uniform sampler2D sampler_Depth;

struct LightData {
    vec4 position; // xyz = pos, w = radius
    vec4 color;    // xyz = color, w = intensity
};

layout(std430, set = 0, binding = 1) readonly buffer LightsListBuffer {
    LightData lights[];
} LightsList;

layout(std430, set = 3, binding = 0) readonly buffer ClustersBuffer {
    vec4 dummy[]; // Not accessed directly in deferred
} ClusterAABB;

layout(std430, set = 3, binding = 2) readonly buffer LightIndicesBuffer {
    uint lightIndexList[];
} LightIndices;

layout(std430, set = 3, binding = 3) readonly buffer ClusterLightOffsetListBuffer {
    uvec2 lightOffsets[];
} ClusterLightOffsetList;

layout(std140, set = 0, binding = 6) uniform ShadowData {
    mat4 lightSpaceMatrices[NUM_CASCADES];
    vec4 cascadeSplits;
} u_Shadow;

layout(set = 0, binding = 7) uniform sampler2D ShadowMaps[NUM_CASCADES];

layout(push_constant) uniform PushConstants {
    vec3 sunLightDir;
};

// --- Light Shading ---
vec3 applyLight(vec3 fragPos, vec3 normal, uint lightIdx) {
    vec3 lightPos = LightsList.lights[lightIdx].position.xyz;
    float radius = LightsList.lights[lightIdx].position.w;
    vec3 color = LightsList.lights[lightIdx].color.rgb;
    float intensity = LightsList.lights[lightIdx].color.w;

    vec3 toLight = lightPos - fragPos;
    float dist = length(toLight);
    vec3 L = normalize(toLight);

    float attenuation = clamp(1.0 - dist / radius, 0.0, 1.0);
    float NdotL = max(dot(normal, L), 0.05);

    return color * intensity * attenuation * NdotL;
}

uint computeClusterIndex(vec3 fragPosView) {
    vec2 fragCoord = gl_FragCoord.xy;
    uvec2 screenSize = uvec2(textureSize(sampler_Position, 0));

    // Cluster XY
    uint tileX = uint(fragCoord.x * float(NUM_CLUSTERS_X) / float(screenSize.x));
    uint tileY = uint(fragCoord.y * float(NUM_CLUSTERS_Y) / float(screenSize.y));
    tileX = clamp(tileX, 0u, NUM_CLUSTERS_X - 1);
    tileY = clamp(tileY, 0u, NUM_CLUSTERS_Y - 1);

    // Cluster Z (logarithmic depth slicing)
    float viewZ = -fragPosView.z; // must be positive in view space
    float z = clamp((log(viewZ) - log(NearZ)) / (log(FarZ) - log(NearZ)), 0.0, 1.0);
    uint tileZ = uint(z * float(NUM_CLUSTERS_Z));
    tileZ = clamp(tileZ, 0u, NUM_CLUSTERS_Z - 1);

    return tileX + tileY * NUM_CLUSTERS_X + tileZ * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;
}

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

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, vec3 worldPos)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec2 coordsXY = projCoords.xy * 0.5 + 0.5;
    float closestDepth = texture(ShadowMaps[0], coordsXY.xy).r; 
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.015);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMaps[0], 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMaps[0], coordsXY.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if (projCoords.z > 1.0 || projCoords.z < 0.0)
        shadow = 0.0;
        
    return shadow;
}

int chooseCascade(vec3 worldPos, mat4 viewMatrix) {
    vec4 viewPos = viewMatrix * vec4(worldPos, 1.0);
    float depth = -viewPos.z; // camera looks along -Z
    for (int i = 0; i < NUM_CASCADES; i++) {
        if (depth < u_Shadow.cascadeSplits[i]) {
            return i;
        }
    }
    return NUM_CASCADES - 1; // farthest cascade
}

vec3 projectToShadowMap(vec3 worldPos, int cascadeIndex) {
    vec4 lightSpacePos = u_Shadow.lightSpaceMatrices[cascadeIndex] * vec4(worldPos, 1.0);
    lightSpacePos.xyz /= lightSpacePos.w;
    vec3 ndc = vec3(lightSpacePos.xy * 0.5 + 0.5, lightSpacePos.z);
    return ndc;
}

float sampleShadow(int cascadeIndex, vec3 shadowCoord) {
    if (shadowCoord.z > 1.0) return 1.0; // behind light frustum, lit
    
    float shadow = 0.0;
    float bias = 0.001; // depth bias to reduce acne
    int samples = 3; // 3x3 kernel
    float texelSize = 1.0 / textureSize(ShadowMaps[cascadeIndex], 0).x;
    
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(ShadowMaps[cascadeIndex], shadowCoord.xy + vec2(x, y) * texelSize).r;
            shadow += (shadowCoord.z - bias <= pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= (samples * samples);
    return shadow;
}

float computeShadow(vec3 worldPos, mat4 viewMatrix) {
    int cascadeIndex = NUM_CASCADES - 1; // farthest cascade
    vec4 viewPos = viewMatrix * vec4(worldPos, 1.0);
    float depth = -viewPos.z; // camera looks along -Z
    for (int i = 0; i < NUM_CASCADES; i++) {
        if (depth < u_Shadow.cascadeSplits[i]) {
            cascadeIndex =  i;
            break;
        }
    }

    vec3 shadowCoord = projectToShadowMap(worldPos, cascadeIndex);
    return sampleShadow(cascadeIndex, shadowCoord);
}

void main() {
    vec3 albedo = texture(sampler_Albedo, uv).rgb;
    float alpha = texture(sampler_Albedo, uv).a;
    vec4 mat = texture(sampler_Material, uv); // Occlusion Roughness Metallic
    float ao = mat.r;
    float roughness = mat.g;
    float metallic = mat.b;
    vec3 N = normalize(texture(sampler_Normal, uv).xyz);
    vec3 emissive = texture(sampler_Emissive, uv).rgb;
    vec3 fragPos = texture(sampler_Position, uv).rgb;

    vec3 fragPosView = vec3(MVP.View * vec4(fragPos, 1.0));
    vec3 V = normalize(MVP.CamPos - fragPos);
    vec3 accum = vec3(0.0);

    uint clusterIdx = computeClusterIndex(fragPosView);

    // Fetch light indices
    uvec2 offsetCount = ClusterLightOffsetList.lightOffsets[clusterIdx];
    uint offset = offsetCount.x;
    uint count = offsetCount.y;

    vec3 LightDir = normalize(-sunLightDir);
    for (uint i = 0; i < count; ++i) {
        uint lightIdx = LightIndices.lightIndexList[offset + i];
        vec3 lightPos = LightsList.lights[lightIdx].position.xyz;
        float radius = LightsList.lights[lightIdx].position.w;
        vec3 lightColor = LightsList.lights[lightIdx].color.rgb;
        float intensity = LightsList.lights[lightIdx].color.w;

        vec3 toLight = lightPos - fragPos;
        float dist = length(toLight);
        vec3 L = normalize(toLight);

        float att = clamp(1.0 - dist/radius, 0.0, 1.0);
        vec3 radiance = lightColor * intensity * att;

        accum += evaluateLight(albedo, metallic, roughness, N, V, L, radiance, ao);
    }

    bool cascadedShadows = true;

    float shadow = 0.0f;
    if(cascadedShadows){
        shadow = computeShadow(fragPos, MVP.View);
    }
    else {
        vec4 fragPosLightSpace = u_Shadow.lightSpaceMatrices[0] * mat4(1.0) * vec4(fragPos, 1.0);
        shadow = (1 - ShadowCalculation(fragPosLightSpace, N, LightDir, fragPos));
    }
    
    // Sun light
    vec3 sunColor = vec3(1.0, 0.95, 0.9);
    float sunIntensity = 6.0;
    vec3 radiance = sunColor * sunIntensity;
    accum +=  shadow * evaluateLight(albedo, metallic, roughness, N, V, LightDir, radiance, ao);

    vec3 ambient = vec3(0.03);
    ambient = ambient * albedo * ao;
    vec3 finalColor = ambient + accum + emissive;

    finalColor = finalColor / (finalColor + vec3(1.0));
    //finalColor = pow(finalColor, vec3(1.0/2.2)); // gamma

    outColor = vec4(finalColor, 1.0);
}
