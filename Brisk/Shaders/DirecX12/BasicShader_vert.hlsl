// VertexShader.hlsl

cbuffer MVPBuffer : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Projection;
    float3 CamPos;
    float padding; // padding to align float3 to 16 bytes
};

struct VSInput {
    float3 inPosition : POSITION;
    float3 inNormal   : NORMAL;
    float2 inUV0      : TEXCOORD0;
    float2 inUV1      : TEXCOORD1;
    float3 inColor    : COLOR;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
    float3 Normal   : TEXCOORD3;
    float3 CamPos   : TEXCOORD4;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    
    float4 worldPos = mul(float4(input.inPosition, 1.0), Model);
    float4 viewPos = mul(worldPos, View);
    float4 projPos = mul(viewPos, Projection);

    output.position = projPos;
    output.UV0 = input.inUV0;
    output.UV1 = input.inUV1;
    output.WorldPos = worldPos.xyz;
    output.Normal = mul((float3x3)Model, input.inNormal); // assuming no non-uniform scale
    output.CamPos = CamPos;

    return output;
}
