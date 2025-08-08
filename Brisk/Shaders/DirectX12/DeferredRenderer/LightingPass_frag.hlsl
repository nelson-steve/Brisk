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

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_Target0
{
    float2 uv = input.uv;

    float3 finalColor = float3(1.0f, 1.0f, 1.0f);

    return float4(finalColor, 1.0f);
}