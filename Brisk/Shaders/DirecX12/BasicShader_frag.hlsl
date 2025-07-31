// PixelShader.hlsl

struct VSOutput {
    float4 position : SV_POSITION;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
    float3 Normal   : TEXCOORD3;
    float3 CamPos   : TEXCOORD4;
};

float4 PSMain(VSOutput input) : SV_Target
{
    float3 lightDir = normalize(float3(1, 1, 1));
    float3 normal = normalize(input.Normal);
    float diffuse = saturate(dot(normal, lightDir));
    
    float3 color = diffuse * float3(1, 0.4, 0.4); // light red diffuse
    return float4(color, 1.0f);
}
