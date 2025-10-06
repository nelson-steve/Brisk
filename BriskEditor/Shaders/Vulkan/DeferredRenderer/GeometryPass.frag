#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint matIndex;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial;
layout(location = 4) out vec4 outEmissive;

layout(set = 1, binding = 0) uniform sampler2D GlobalTextures[];

struct MaterialData {
    int alphaMode;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;

    float ior;
    float dispersion;
    int doubleSided;
    int unlit;

    float emissiveStrength;
    vec4 baseColorFactor;
    vec3 emissiveFactor;

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

    vec3 sheenColorFactor;

    int sheenColorTextureIndex;
    int sheenColorTextureUV;
    float sheenRoughnessFactor;
    int sheenRoughnessTextureIndex;
    int sheenRoughnessTextureUV;

    float specularFactor;
    int specularTextureIndex;
    int specularTextureUV;

    vec3 specularColorFactor;
    int specularColorTextureIndex;
    int specularColorTextureUV;

    float transmissionFactor;
    int transmissionTextureIndex;
    int transmissionTextureUV;

    float thicknessFactor;
    int thicknessTextureIndex;
    int thicknessTextureUV;
    float attenuationDistance;

    vec3 attenuationColor;
};

layout(std430, set = 0, binding = 2) readonly buffer MaterialsBuffer {
    MaterialData materials[];
} Materials;

void main() {
    MaterialData material = Materials.materials[0];

    // Base Color
    vec4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureIndex != -1) {
        baseColor *= texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], fragUV);

        if (material.alphaMode == 1) { // MASK
            if (baseColor.a < material.alphaCutoff) {
                //discard;
            }
        } else if (material.alphaMode == 2) { // BLEND
            outAlbedo.a = baseColor.a; // Pass alpha to lighting pass
            // You typically render this in a **separate transparent pass**.
            //return; // Skip writing to G-buffer in opaque pass
        }
    }

    // Position and normal
    outPosition = vec4(fragPosition, 1.0);
    vec3 normal = fragNormal;

    // Metallic & Roughness
    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != -1) {
        vec4 mrSample = texture(GlobalTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], fragUV);
        metallic *= mrSample.b;
        roughness *= mrSample.g;
    }

    // Normal map (basic unpack)
    if (material.normalTextureIndex != -1) {
        vec3 tangentNormal = texture(GlobalTextures[nonuniformEXT(material.normalTextureIndex)], fragUV).xyz;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        normal = normalize(tangentNormal); // Ideally transform to world-space if using tangent-space normals
    }

    // Emissive
    outEmissive = vec4(material.emissiveFactor, 1.0);
    if (material.emissiveTextureIndex != -1) {
        outEmissive *= texture(GlobalTextures[nonuniformEXT(material.emissiveTextureIndex)], fragUV);
    }
    outEmissive *= material.emissiveStrength;

    // Occlusion (for lighting pass)
    float occlusion = 1.0;
    if (material.occlusionTextureIndex != -1) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], fragUV).r;
    }

    // G-buffer outputs
    outNormal = vec4(normalize(normal), 1.0);
    outAlbedo = baseColor;
    outMaterial = vec4(metallic, roughness, occlusion, float(material.unlit));
}