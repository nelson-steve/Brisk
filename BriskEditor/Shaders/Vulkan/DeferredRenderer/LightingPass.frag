#version 450

#define NUM_CLUSTERS_X 16
#define NUM_CLUSTERS_Y 9
#define NUM_CLUSTERS_Z 24
#define NUM_TOTAL_CLUSTERS (NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z)

#define ScreenX 1920.0
#define ScreenY 1080.0
#define NearZ 1.0
#define FarZ 1000.0

#define NUM_CASCADES 4

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

vec3 applyDirectionalLight(vec3 fragPos, vec3 normal) {
    float dirLightIntensity = 1.0;
    vec3 dirLightColor = vec3(1.0);
    // vec3 dirLightDir = normalize(dirLightPos - fragPos);
    vec3 L = normalize(-sunLightDir); // light shines along -dirLightDir
    float NdotL = max(dot(normal, L), 0.0);
    return dirLightColor * dirLightIntensity * NdotL;
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
    // Perspective divide
    lightSpacePos.xyz /= lightSpacePos.w;
    // Convert from [-1,1] NDC to [0,1] UV
    return lightSpacePos.xyz * 0.5 + 0.5;
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
    int cascadeIndex = chooseCascade(worldPos, viewMatrix);
    vec3 shadowCoord = projectToShadowMap(worldPos, cascadeIndex);
    return sampleShadow(cascadeIndex, shadowCoord);
}

// --- Main ---
void main() {
    vec3 fragPos = texture(sampler_Position, uv).rgb;
    vec3 normal = normalize(texture(sampler_Normal, uv).rgb);
    vec3 albedo = texture(sampler_Albedo, uv).rgb;
    float alpha = texture(sampler_Albedo, uv).a;
    vec3 emissive = texture(sampler_Emissive, uv).rgb;

    vec3 fragPosView = vec3(MVP.View * vec4(fragPos, 1.0));
    uint clusterIdx = computeClusterIndex(fragPosView);

    // Fetch light indices
    uvec2 offsetCount = ClusterLightOffsetList.lightOffsets[clusterIdx];
    uint offset = offsetCount.x;
    uint count = offsetCount.y;

    vec3 litColor = vec3(0.0);
    for (uint i = 0; i < 2048; ++i) {
        uint lightIdx = LightIndices.lightIndexList[offset + i];
        litColor += applyLight(fragPos, normal, i);
    }

    //float shadow = computeShadow(fragPos, MVP.View);
    //litColor *= shadow;

    vec3 ambient = 0.3 * albedo;
    vec3 finalColor = ambient + litColor * albedo + emissive;

    outColor = vec4(finalColor, 1.0);

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
