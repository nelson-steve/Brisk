#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage  : require
#extension GL_GOOGLE_include_directive: require

#include "Mesh.hpp"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;
layout(location = 3) flat in uint drawId;
layout(location = 4) in vec4 Tangent;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial; // Occlusion Roughness Metallic
layout(location = 4) out vec4 outEmissive;

layout(set = 1, binding = 0) uniform sampler2D GlobalTextures[];

struct MaterialData {
    uint alphaMode;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;

    vec4 baseColorFactor;
    vec3 emissiveFactor;

    float emissiveStrength;

    int baseColorTextureIndex;
    int metallicRoughnessTextureIndex;
    int normalTextureIndex;
    int occlusionTextureIndex;
    int emissiveTextureIndex;
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
    if (material.baseColorTextureIndex != -1) {
        baseColor *= texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], UV);

        if (material.alphaMode == 1) { // MASK
            if (baseColor.a < material.alphaCutoff) {
                discard;
            }
        } else if (material.alphaMode == 2) { // BLEND
            outAlbedo.a = baseColor.a;
            return;
        }
    }

    outPosition = vec4(Position, 1.0);
    vec3 Ng = normalize(Normal);
    vec3 Ns = Ng;

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if (material.metallicRoughnessTextureIndex != -1) {
        vec4 textureSample = texture(GlobalTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], UV);
        metallic *= textureSample.b;
        roughness *= textureSample.g;
    }

    bool hasValidTangent = length(Tangent.xyz) > 0.0001;
    if (material.normalTextureIndex != -1 && hasValidTangent) {
        // [0,1] -> [-1,1]
        vec3 tangentNormal =
            texture(GlobalTextures[nonuniformEXT(material.normalTextureIndex)], UV).xyz
            * 2.0 - 1.0;
        
        vec3 T = normalize(Tangent.xyz);
        T = normalize(T - Ng * dot(Ng, T));
        vec3 B = cross(Ng, T) * Tangent.w;
        
        mat3 TBN = mat3(T, B, Ng);
        
        Ns = normalize(TBN * tangentNormal);
        
        // Clamp invalid normals
        if (dot(Ns, Ng) < 0.0)
            Ns = Ng;
    }

    outEmissive = vec4(material.emissiveFactor, 1.0);
    if (material.emissiveTextureIndex != -1) {
        outEmissive *= texture(GlobalTextures[nonuniformEXT(material.emissiveTextureIndex)], UV);
    }
    outEmissive *= material.emissiveStrength;

    float occlusion = 1.0;
    if (material.occlusionTextureIndex != -1) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], UV).r;
    }

    outNormal = vec4(Ns * 0.5 + 0.5, 1.0);
    outAlbedo = baseColor;

    outMaterial = vec4(occlusion, roughness, metallic, 1.0);
}