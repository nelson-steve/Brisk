cbuffer MVPBuffer : register(b0) {
    float4x4 projView;
    float3 CamPos;
    float padding;
};

struct MeshData {
    float4x4 model;
    uint materialIndex;
    uint padding[3];
};

ConstantBuffer<MeshData> meshData : register(b1);

struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 color : COLOR;
    float4 tangent : TANGENT;
    uint4  jointWeights : JOINTWEIGHTS;
    float4 jointIndices : JOINTINDICES;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = mul(meshData.model, float4(input.position, 1.0f));
    float3x3 model3x3 = (float3x3)meshData.model;
    float3x3 normalMatrix = transpose(model3x3);
    output.normal = normalize(mul(normalMatrix, input.normal));
    output.uv = input.uv0;
    return output;    
}