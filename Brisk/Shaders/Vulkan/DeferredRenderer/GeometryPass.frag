#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial;

layout(set = 3, binding = 0) uniform sampler2D global_textures[];

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

    uint baseColorTextureIndex;
    uint baseColorTextureUV;
    uint metallicRoughnessTextureIndex;
    uint metallicRoughnessTextureUV;

    uint normalTextureIndex;
    uint normalTextureUV;
    uint occlusionTextureIndex;
    uint occlusionTextureUV;

    uint emissiveTextureIndex;
    uint emissiveTextureUV;

    float anisotropyStrength;
    float anisotropyRotation;
    uint anisotropyTextureIndex;
    uint anisotropyTextureUV;

    float clearcoatFactor;
    uint clearcoatTextureIndex;
    uint clearcoatTextureUV;
    float clearcoatRoughnessFactor;

    uint clearcoatRoughnessTextureIndex;
    uint clearcoatRoughnessTextureUV;
    uint clearcoatNormalTextureIndex;
    uint clearcoatNormalTextureUV;

    float iridescenceFactor;
    uint iridescenceTextureIndex;
    uint iridescenceTextureUV;
    float iridescenceIor;

    float iridescenceThicknessMinimum;
    float iridescenceThicknessMaximum;
    uint iridescenceThicknessTextureIndex;
    uint iridescenceThicknessTextureUV;

    vec3 sheenColorFactor;

    uint sheenColorTextureIndex;
    uint sheenColorTextureUV;
    float sheenRoughnessFactor;
    uint sheenRoughnessTextureIndex;
    uint sheenRoughnessTextureUV;

    float specularFactor;
    uint specularTextureIndex;
    uint specularTextureUV;

    vec3 specularColorFactor;
    uint specularColorTextureIndex;
    uint specularColorTextureUV;

    float transmissionFactor;
    uint transmissionTextureIndex;
    uint transmissionTextureUV;

    float thicknessFactor;
    uint thicknessTextureIndex;
    uint thicknessTextureUV;
    float attenuationDistance;

    vec3 attenuationColor;
};

layout(std430, set = 4, binding = 0) readonly buffer SSBO {
    MaterialData materials[];
};

layout(push_constant) uniform PushConstants {
    uint materialIndex;
} pc;

void main() {
    MaterialData material = materials[pc.materialIndex];

    // Position and normal
    outPosition = vec4(fragPosition, 1.0);
    vec3 normal = normalize(fragNormal);

    // Base Color
    vec4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureIndex != -1) {
        baseColor *= texture(global_textures[nonuniformEXT(material.baseColorTextureIndex)], fragUV);
    }

    // Metallic & Roughness
    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != -1) {
        vec4 mrSample = texture(global_textures[nonuniformEXT(material.metallicRoughnessTextureIndex)], fragUV);
        metallic *= mrSample.b;
        roughness *= mrSample.g;
    }

    // Normal map (basic unpack)
    if (material.normalTextureIndex != -1) {
        vec3 tangentNormal = texture(global_textures[nonuniformEXT(material.normalTextureIndex)], fragUV).xyz;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        normal = normalize(tangentNormal); // Ideally transform to world-space if using tangent-space normals
    }

    // Emissive
    vec3 emissive = material.emissiveFactor;
    if (material.emissiveTextureIndex != -1) {
        emissive *= texture(global_textures[nonuniformEXT(material.emissiveTextureIndex)], fragUV).rgb;
    }
    emissive *= material.emissiveStrength;

    // Occlusion (for lighting pass)
    float occlusion = 1.0;
    if (material.occlusionTextureIndex != -1) {
        occlusion = texture(global_textures[nonuniformEXT(material.occlusionTextureIndex)], fragUV).r;
    }

    // G-buffer outputs
    outNormal = vec4(normalize(normal), 1.0);
    outAlbedo = baseColor;
    outMaterial = vec4(metallic, roughness, occlusion, float(material.unlit));
}