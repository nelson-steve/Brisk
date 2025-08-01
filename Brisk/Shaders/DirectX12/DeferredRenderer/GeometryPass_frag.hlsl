cbuffer MeshData : register(b0)
{
    float4x4 model;
    int materialIndex;
};

Texture2D u_GlobalTextures[512] : register(t0);
SamplerState sampler0 : register(s0);

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

StructuredBuffer<MaterialData> ssbo_Materials : register(t128, space1);

struct PSInput
{
    float3 position : TEXCOORD0;
    float3 normal   : TEXCOORD1;
    float2 uv       : TEXCOORD2;
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

    MaterialData material = ssbo_Materials[materialIndex];

    float4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureIndex != -1)
    {
        baseColor *= u_GlobalTextures[material.baseColorTextureIndex].Sample(sampler0, input.uv);

        if (material.alphaMode == 1 && baseColor.a < material.alphaCutoff)
        {
            discard;
        }
        else if (material.alphaMode == 2)
        {
            output.albedo.a = baseColor.a;
            discard; // skip writing to G-buffer
        }
    }

    float3 normal = normalize(input.normal);
    if (material.normalTextureIndex != -1)
    {
        float3 tangentNormal = u_GlobalTextures[material.normalTextureIndex].Sample(sampler0, input.uv).xyz;
        tangentNormal = tangentNormal * 2.0f - 1.0f;
        normal = normalize(tangentNormal); // ideally TBN transform here
    }

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != -1)
    {
        float4 mrSample = u_GlobalTextures[material.metallicRoughnessTextureIndex].Sample(sampler0, input.uv);
        metallic *= mrSample.b;
        roughness *= mrSample.g;
    }

    float occlusion = 1.0f;
    if (material.occlusionTextureIndex != -1)
    {
        occlusion = u_GlobalTextures[material.occlusionTextureIndex].Sample(sampler0, input.uv).r;
    }

    float4 emissive = float4(material.emissiveFactor, 1.0f);
    if (material.emissiveTextureIndex != -1)
    {
        emissive *= u_GlobalTextures[material.emissiveTextureIndex].Sample(sampler0, input.uv);
    }
    emissive *= material.emissiveStrength;

    output.position = float4(input.position, 1.0f);
    output.normal = float4(normal, 1.0f);
    output.albedo = baseColor;
    output.material = float4(metallic, roughness, occlusion, float(material.unlit));
    output.emissive = emissive;

    return output;
}