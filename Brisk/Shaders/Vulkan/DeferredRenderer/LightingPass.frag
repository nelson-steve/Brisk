#version 450

#define MAX_LIGHTS 2048
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS_X 16
#define NUM_CLUSTERS_Y 9
#define NUM_CLUSTERS_Z 24
#define NUM_TOTAL_CLUSTERS (NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z)

#define ScreenX 1920.0
#define ScreenY 1080.0
#define NearZ 0.1
#define FarZ 100.0

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform MVP {
    mat4 ProjView;
    mat4 View;
    vec3 CamPos;
    float _padding0;
} u_MVP;

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

layout(std430, set = 1, binding = 0) readonly buffer LightsList {
    LightData lights[];
} ssbo_LightsList;

layout(std430, set = 5, binding = 0) readonly buffer Clusters {
    vec4 dummy[]; // Not accessed directly in deferred
} ssbo_ClusterAABB;

layout(std430, set = 5, binding = 2) readonly buffer LightIndices {
    uint lightIndexList[];
} ssbo_LightIndices;

layout(std430, set = 5, binding = 3) readonly buffer ClusterLightOffsetList {
    uvec2 lightOffsets[];
} ssbo_ClusterLightOffsetList;

// --- Light Shading ---
vec3 applyLight(vec3 fragPos, vec3 normal, uint lightIdx) {
    vec3 lightPos = ssbo_LightsList.lights[lightIdx].position.xyz;
    float radius = ssbo_LightsList.lights[lightIdx].position.w;
    vec3 color = ssbo_LightsList.lights[lightIdx].color.rgb;
    float intensity = ssbo_LightsList.lights[lightIdx].color.w;

    vec3 toLight = lightPos - fragPos;
    float dist = length(toLight);
    vec3 L = normalize(toLight);

    float attenuation = clamp(1.0 - dist / radius, 0.0, 1.0);
    float NdotL = max(dot(normal, L), 0.05);

    return color * intensity * attenuation * NdotL;
}

uint computeClusterIndex(vec3 fragPosView) {
    vec2 fragCoord = gl_FragCoord.xy;

    // Compute cluster XY
    uvec3 tileSize = uvec3(16, 9, 24);
    uvec2 screenSize = uvec2(1920, 1080);

    uint tileX = uint(fragCoord.x * float(NUM_CLUSTERS_X) / screenSize.x);
    uint tileY = uint(fragCoord.y * float(NUM_CLUSTERS_Y) / screenSize.y);

    // Compute cluster Z (logarithmic depth slicing)
    float viewZ = -fragPosView.z;
    float z = clamp((log(viewZ) - log(NearZ)) / (log(FarZ) - log(NearZ)), 0.0, 1.0);
    uint tileZ = uint(z * float(NUM_CLUSTERS_Z));

    tileX = clamp(tileX, 0u, NUM_CLUSTERS_X - 1);
    tileY = clamp(tileY, 0u, NUM_CLUSTERS_Y - 1);
    tileZ = clamp(tileZ, 0u, NUM_CLUSTERS_Z - 1);

    return tileX + tileY * NUM_CLUSTERS_X + tileZ * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;
}

// --- Main ---
void main() {
    vec3 fragPos = texture(sampler_Position, uv).rgb;
    vec3 normal = normalize(texture(sampler_Normal, uv).rgb);
    vec3 albedo = texture(sampler_Albedo, uv).rgb;
    float alpha = texture(sampler_Albedo, uv).a;
    vec3 emissive = texture(sampler_Emissive, uv).rgb;

    vec3 fragPosView = vec3(u_MVP.View * vec4(fragPos, 1.0));
    uint clusterIdx = computeClusterIndex(fragPosView);

    // Fetch light indices
    uvec2 offsetCount = ssbo_ClusterLightOffsetList.lightOffsets[clusterIdx];
    uint offset = offsetCount.x;
    uint count = offsetCount.y;

    vec3 litColor = vec3(0.0);
    for (uint i = 0; i < count; ++i) {
        uint lightIdx = ssbo_LightIndices.lightIndexList[offset + i];
        litColor += applyLight(fragPos, normal, lightIdx);
    }

    vec3 ambient = 0.3 * albedo;
    vec3 finalColor = ambient + litColor * albedo + emissive;

    outColor = vec4(finalColor, alpha);

    // Visualize number of lights in this tile
    //float brightness = float(count) / float(MAX_LIGHTS_PER_CLUSTER);
    //outColor = vec4(vec3(brightness), 1.0);

    // Debug: visualize tile index as color
    //vec3 debugColor = vec3(
    //    float(xSlice) / float(NUM_CLUSTERS_X),
    //    float(ySlice) / float(NUM_CLUSTERS_Y),
    //    float(zSlice) / float(NUM_CLUSTERS_Z)
    //);
    //outColor = vec4(debugColor, 1.0);
}
