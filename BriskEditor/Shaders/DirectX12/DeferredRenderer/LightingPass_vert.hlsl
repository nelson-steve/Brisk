struct VSOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;    
};

VSOutput VSMain(uint vertexID : SV_VertexID) {
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };    
    VSOutput output;
    output.position = float4(positions[vertexID], 0.0, 1.0);
    output.uv = (positions[vertexID] + 1.0) * 0.5;
    return output;
}