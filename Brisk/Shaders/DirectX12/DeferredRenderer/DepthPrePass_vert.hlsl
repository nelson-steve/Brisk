cbuffer MVPBuffer : register(b0, space0) {
    float4x4 projView;
    float4x4 View;
    float3 CamPos;
    float padding;
};

struct VSInput {
    float3 position : POSITION;
};

struct VSOutput {
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = mul(projView, float4(input.position, 1.0f));
    return output;
}