#define MAX_LIGHTS 2048
#define MAX_LIGHTS_PER_CLUSTER 128
#define NUM_CLUSTERS_X 16
#define NUM_CLUSTERS_Y 9
#define NUM_CLUSTERS_Z 24
#define NUM_TOTAL_CLUSTERS (NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z)

#define ScreenX 1920.0
#define ScreenY 1080.0
#define NearZ 0.1
#define FarZ 1000.0

cbuffer MVP : register(b0, space0)
{
    float4x4 projView;
    float3 CamPos;
    float padding;
};

struct MeshData
{
    float4x4 model;
    uint materialIndex;
};

// G-buffer textures
Texture2D sampler_Position : register(t0, space2);
Texture2D sampler_Normal   : register(t1, space2);
Texture2D sampler_Albedo   : register(t2, space2);
Texture2D sampler_Material : register(t3, space2);
Texture2D sampler_Emissive : register(t4, space2);
Texture2D sampler_Depth    : register(t5, space2);
SamplerState defaultSampler : register(s0, space2);

// Light data
struct LightData {
    float4 position; // xyz = pos, w = radius
    float4 color;    // xyz = color, w = intensity
};
StructuredBuffer<LightData> lights : register(t0, space1);

// Light index/cluster data
//StructuredBuffer<uint> lightIndexList : register(t2, space5);
//StructuredBuffer<uint2> lightOffsets  : register(t3, space5);

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float3 applyLight(float3 fragPos, float3 normal, uint lightIdx)
{
    float3 lightPos = lights[lightIdx].position.xyz;
    float radius = lights[lightIdx].position.w;
    float3 color = lights[lightIdx].color.rgb;
    float intensity = lights[lightIdx].color.w;

    float3 toLight = lightPos - fragPos;
    float dist = length(toLight);
    float3 L = normalize(toLight);

    float attenuation = saturate(1.0 - dist / radius);
    float NdotL = max(dot(normal, L), 0.05);

    return color * intensity * attenuation * NdotL;
}

uint computeClusterIndex(float3 fragPosView, float2 fragCoord)
{
    uint tileX = uint(fragCoord.x * float(NUM_CLUSTERS_X) / ScreenX);
    uint tileY = uint(fragCoord.y * float(NUM_CLUSTERS_Y) / ScreenY);

    float viewZ = -fragPosView.z;
    float z = clamp((log(viewZ) - log(NearZ)) / (log(FarZ) - log(NearZ)), 0.0, 1.0);
    uint tileZ = uint(z * float(NUM_CLUSTERS_Z));

    tileX = clamp(tileX, 0u, NUM_CLUSTERS_X - 1);
    tileY = clamp(tileY, 0u, NUM_CLUSTERS_Y - 1);
    tileZ = clamp(tileZ, 0u, NUM_CLUSTERS_Z - 1);

    return tileX + tileY * NUM_CLUSTERS_X + tileZ * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;
}

float4 PSMain(PSInput input) : SV_Target0
{
    float2 uv = input.uv;

    float3 fragPos = sampler_Position.Sample(defaultSampler, uv).rgb;
    float3 normal = normalize(sampler_Normal.Sample(defaultSampler, uv).rgb);
    float4 albedoSample = sampler_Albedo.Sample(defaultSampler, uv);
    float3 albedo = albedoSample.rgb;
    float alpha = albedoSample.a;
    float3 emissive = sampler_Emissive.Sample(defaultSampler, uv).rgb;

    //float4 viewFrag = mul(View, float4(fragPos, 1.0));
    //float3 fragPosView = viewFrag.xyz;

    float2 fragCoord = input.pos.xy;
    //uint clusterIdx = computeClusterIndex(fragPosView, fragCoord);

    //uint2 offsetCount = lightOffsets[clusterIdx];
    //uint offset = offsetCount.x;
    //uint count = offsetCount.y;

    float3 litColor = 0.0f;
    //for (uint i = 0; i < count; ++i)
    //{
    //    uint lightIdx = lightIndexList[offset + i];
    //    litColor += applyLight(fragPos, normal, lightIdx);
    //}

    float3 ambient = 0.3f * albedo;
    float3 finalColor = ambient + litColor * albedo + emissive;

    return float4(finalColor, alpha);
}