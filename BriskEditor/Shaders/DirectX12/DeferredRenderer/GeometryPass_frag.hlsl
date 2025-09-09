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

struct MaterialData
{
    int alphaMode;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;

    float ior;
    float dispersion;
    int doubleSided;
    int unlit;

    float emissiveStrength;
    float4 baseColorFactor;
    float3 emissiveFactor;

    int baseColorTextureIndex;
    int baseColorTextureUV;
    int metallicRoughnessTextureIndex;
    int metallicRoughnessTextureUV;

    int normalTextureIndex;
    int normalTextureUV;
    int occlusionTextureIndex;
    int occlusionTextureUV;

    int emissiveTextureIndex;
    int emissiveTextureUV;

    float anisotropyStrength;
    float anisotropyRotation;
    int anisotropyTextureIndex;
    int anisotropyTextureUV;

    float clearcoatFactor;
    int clearcoatTextureIndex;
    int clearcoatTextureUV;
    float clearcoatRoughnessFactor;

    int clearcoatRoughnessTextureIndex;
    int clearcoatRoughnessTextureUV;
    int clearcoatNormalTextureIndex;
    uint clearcoatNormalTextureUV;

    float iridescenceFactor;
    int iridescenceTextureIndex;
    int iridescenceTextureUV;
    float iridescenceIor;

    float iridescenceThicknessMinimum;
    float iridescenceThicknessMaximum;
    int iridescenceThicknessTextureIndex;
    int iridescenceThicknessTextureUV;

    float3 sheenColorFactor;

    int sheenColorTextureIndex;
    int sheenColorTextureUV;
    float sheenRoughnessFactor;
    int sheenRoughnessTextureIndex;
    int sheenRoughnessTextureUV;

    float specularFactor;
    int specularTextureIndex;
    int specularTextureUV;

    float3 specularColorFactor;
    int specularColorTextureIndex;
    int specularColorTextureUV;

    float transmissionFactor;
    int transmissionTextureIndex;
    int transmissionTextureUV;

    float thicknessFactor;
    int thicknessTextureIndex;
    int thicknessTextureUV;
    float attenuationDistance;

    float3 attenuationColor;
};

StructuredBuffer<MaterialData> Materials : register(t0, space0);

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    output.position = input.position;
    output.normal = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.material = float4(1.0f, 1.0f, 1.0f, Materials[0].metallicFactor);
    output.emissive = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}