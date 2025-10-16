#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage  : require
#extension GL_GOOGLE_include_directive: require

#include "Mesh.hpp"

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint drawId;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial; // Occlusion Roughness Metallic
layout(location = 4) out vec4 outEmissive;

layout(set = 1, binding = 0) uniform sampler2D GlobalTextures[];

struct MaterialData {
    uint alphaMode; // use 4 bytes to avoid packing issues
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;

    vec4 baseColorFactor;
    vec3 emissiveFactor;

    float emissiveStrength;

    uint baseColorTextureIndex;
    uint metallicRoughnessTextureIndex;
    uint normalTextureIndex;
    uint occlusionTextureIndex;
    uint emissiveTextureIndex;
};

layout(std430, set = 0, binding = 2) readonly buffer MaterialsBuffer {
    MaterialData materials[];
} Materials;

layout(set = 0, binding = 9) readonly buffer MeshDrawsBuffer {
	MeshDraw meshDraws[];
} MeshDraws;

void main() {
    MeshDraw draw = MeshDraws.meshDraws[drawId];
    MaterialData material = Materials.materials[draw.materialIndex];

    vec4 baseColor = material.baseColorFactor;
    if (material.baseColorTextureIndex != 0) {
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

    outPosition = vec4(fragPosition, 1.0);
    vec3 normal = fragNormal;

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != 0) {
        vec4 textureSample = texture(GlobalTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], fragUV);
        metallic *= textureSample.b;
        roughness *= textureSample.g;
    }

    if (material.normalTextureIndex != 0) {
        vec3 tangentNormal = texture(GlobalTextures[nonuniformEXT(material.normalTextureIndex)], fragUV).xyz;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        normal = normalize(tangentNormal); // Ideally transform to world-space if using tangent-space normals
    }

    outEmissive = vec4(material.emissiveFactor, 1.0);
    if (material.emissiveTextureIndex != 0) {
        outEmissive *= texture(GlobalTextures[nonuniformEXT(material.emissiveTextureIndex)], fragUV);
    }
    outEmissive *= material.emissiveStrength;

    float occlusion = 1.0;
    if (material.occlusionTextureIndex != 0) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], fragUV).r;
    }

    outNormal = vec4(normalize(normal), 1.0);
    outAlbedo = baseColor;
    outMaterial = vec4(occlusion, roughness, metallic, 1.0);
}