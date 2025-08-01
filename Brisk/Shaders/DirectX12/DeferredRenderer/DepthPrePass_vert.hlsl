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
};

struct VSOutput {
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = mul(projView, mul(meshData.model, float4(input.position, 1.0f)));
    return output;
}