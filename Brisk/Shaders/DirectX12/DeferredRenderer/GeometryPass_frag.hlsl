cbuffer MeshData : register(b0)
{
    float4x4 model;
    int materialIndex;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

struct PSOutput
{
    float4 position : SV_Target0;
    float4 normal   : SV_Target1;
    float4 albedo   : SV_Target2;
    float4 material : SV_Target3;
    float4 emissive : SV_Target4;
};

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    output.position = input.position;
    output.normal = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.material = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.emissive = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}