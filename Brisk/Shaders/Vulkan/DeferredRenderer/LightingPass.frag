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

// --- Main ---
void main() {
    vec3 fragPos = texture(sampler_Position, uv).rgb;
    vec3 normal = normalize(texture(sampler_Normal, uv).rgb);
    vec3 albedo = texture(sampler_Albedo, uv).rgb;
    float alpha = texture(sampler_Albedo, uv).a;
    vec3 emissive = texture(sampler_Emissive, uv).rgb;

    // Log depth clustering
    float viewZ = -fragPos.z; // view space Z is negative
    float logDepthMin = log2(NearZ);
    float logDepthMax = log2(FarZ);
    float scale = float(NUM_CLUSTERS_Z) / (logDepthMax - logDepthMin);
    float bias  = -logDepthMin * scale;

    float zSliceF = log2(max(viewZ, 0.0001)) * scale + bias;
    uint zSlice = uint(clamp(zSliceF, 0.0, float(NUM_CLUSTERS_Z - 1)));

    uint xSlice = uint(clamp(gl_FragCoord.x / ScreenX * float(NUM_CLUSTERS_X), 0.0, float(NUM_CLUSTERS_X - 1)));
    uint ySlice = uint(clamp(gl_FragCoord.y / ScreenY * float(NUM_CLUSTERS_Y), 0.0, float(NUM_CLUSTERS_Y - 1)));

    uint tileIndex = xSlice + ySlice * NUM_CLUSTERS_X + zSlice * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;

    // Fetch light indices
    uvec2 offsetCount = ssbo_ClusterLightOffsetList.lightOffsets[tileIndex];
    uint offset = offsetCount.x;
    uint count = offsetCount.y;

    vec3 litColor = vec3(0.0);
    for (uint i = 0; i < count; ++i) {
        uint lightIdx = ssbo_LightIndices.lightIndexList[offset + i];
        litColor += applyLight(fragPos, normal, lightIdx);
    }

    vec3 ambient = 0.3 * albedo;
    vec3 finalColor = ambient + litColor * albedo + emissive;

    //outColor = vec4(finalColor, alpha);

    // Visualize number of lights in this tile
    //float brightness = float(count) / float(MAX_LIGHTS_PER_CLUSTER);
    //outColor = vec4(vec3(brightness), 1.0);

    // Debug: visualize tile index as color
    vec3 debugColor = vec3(
        float(xSlice) / float(NUM_CLUSTERS_X),
        float(ySlice) / float(NUM_CLUSTERS_Y),
        float(zSlice) / float(NUM_CLUSTERS_Z)
    );
    outColor = vec4(debugColor, 1.0);
}
