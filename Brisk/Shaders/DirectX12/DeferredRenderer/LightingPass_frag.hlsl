cbuffer MVPBuffer : register(b0, space0) {
    float4x4 projView;
    float4x4 View;
    float3 CamPos;
    float padding;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D sampler_Position : register(t0, space2);
Texture2D sampler_Normal   : register(t1, space2);
Texture2D sampler_Albedo   : register(t2, space2);
Texture2D sampler_Material : register(t3, space2);
Texture2D sampler_Emissive : register(t4, space2);
Texture2D sampler_Depth    : register(t5, space2);
SamplerState defaultSampler : register(s0, space2);

struct LightData {
    float4 position; // xyz = pos, w = radius
    float4 color;    // xyz = color, w = intensity
};
StructuredBuffer<LightData> lights : register(t0, space1);

float4 PSMain(PSInput input) : SV_Target0
{
    float2 uv = input.uv;

    float3 fragPos = sampler_Position.Sample(defaultSampler, uv).rgb;
    float3 normal = normalize(sampler_Normal.Sample(defaultSampler, uv).rgb);
    float4 albedoSample = sampler_Albedo.Sample(defaultSampler, uv);
    float3 albedo = albedoSample.rgb;
    float alpha = albedoSample.a;
    float3 emissive = sampler_Emissive.Sample(defaultSampler, uv).rgb;    

    float3 tmp = sampler_Normal.Sample(defaultSampler, uv).rgb;
    tmp += sampler_Albedo.Sample(defaultSampler, uv).rgb;
    tmp += sampler_Material.Sample(defaultSampler, uv).rgb;
    tmp += sampler_Emissive.Sample(defaultSampler, uv).rgb;
    tmp += sampler_Depth.Sample(defaultSampler, uv).rrr;
    tmp += lights[0].position.rgb;

    float3 finalColor = float3(1.0f, 1.0f, 1.0f);

    return float4(tmp, 1.0f);
}