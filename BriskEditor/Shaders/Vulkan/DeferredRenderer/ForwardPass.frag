#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage  : require
#extension GL_GOOGLE_include_directive: require

#define PI 3.14159265359

#include "../config.hpp"
#include "Mesh.hpp"

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;
layout(location = 3) flat in uint drawId;
layout(location = 4) in vec4 Tangent;

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) uniform MVPBuffer {
    mat4 ProjView;
    mat4 View;
    vec3 CamPos;
} MVP;

layout(std140, set = 0, binding = 21) uniform RendererSettingBuffer {
    float knee;
    float threshold;
    float intensity;
    uint csm;
    uint pcf;
    float pcfScale;
} RendererSetting;

struct LightData {
    vec4 position; // xyz = pos, w = radius
    vec4 color;    // xyz = color, w = intensity
};

layout(std430, set = 0, binding = 1) readonly buffer LightsListBuffer {
    LightData lights[];
} LightsList;

layout(std430, set = 3, binding = 2) readonly buffer LightIndicesBuffer {
    uint lightIndexList[];
} LightIndices;

layout(std430, set = 3, binding = 3) readonly buffer ClusterLightOffsetListBuffer {
    uvec2 lightOffsets[];
} ClusterLightOffsetList;

layout(std140, set = 0, binding = 6) uniform ShadowData {
    mat4 lightSpaceMatrices[SHADOW_MAP_CASCADE_COUNT];
    vec4 cascadeSplits;
} u_Shadow;

layout(set = 1, binding = 0) uniform sampler2D GlobalTextures[];

struct MaterialData {
    uint alphaMode;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    bool doubleSided;

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

layout(set = 0, binding = 24) readonly buffer MeshDrawsBlendedBuffer {
	MeshDraw meshDraws[];
} MeshDrawsBlended;

layout(set = 0, binding = 7) uniform sampler2D ShadowMaps[SHADOW_MAP_CASCADE_COUNT];

layout(push_constant) uniform PushConstants {
    vec3 sunLightDir;
};

uint computeClusterIndex(vec3 fragPosView) {
    vec2 fragCoord = gl_FragCoord.xy;
    //uvec2 screenSize = uvec2(textureSize(sampler_Position, 0));
    uvec2 screenSize = uvec2(1920, 1080);

    // Cluster XY
    uint tileX = uint(fragCoord.x * float(NUM_CLUSTERS_X) / float(screenSize.x));
    uint tileY = uint(fragCoord.y * float(NUM_CLUSTERS_Y) / float(screenSize.y));
    tileX = clamp(tileX, 0u, NUM_CLUSTERS_X - 1);
    tileY = clamp(tileY, 0u, NUM_CLUSTERS_Y - 1);

    // Cluster Z (logarithmic depth slicing)
    float viewZ = -fragPosView.z; // must be positive in view space
    float z = clamp((log(viewZ) - log(NearZ)) / (log(FarZ) - log(NearZ)), 0.0, 1.0);
    uint tileZ = uint(z * float(NUM_CLUSTERS_Z));
    tileZ = clamp(tileZ, 0u, NUM_CLUSTERS_Z - 1);

    return tileX + tileY * NUM_CLUSTERS_X + tileZ * NUM_CLUSTERS_X * NUM_CLUSTERS_Y;
}

// Trowbridge-Reitz / GGX
float DGGX(float NdotH, float roughness) {
    float a2 = roughness * roughness;
    float a2_ = a2 * a2;
    float denom = (NdotH * NdotH) * (a2_ - 1.0) + 1.0;
    return a2_ / (PI * denom * denom);
}

// Schlick-GGX (Smith)
float GSchlickGGX(float NdotV, float k) {
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GSmith(float NdotV, float NdotL, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return GSchlickGGX(NdotV, k) * GSchlickGGX(NdotL, k);
}

// Schlick approximation
vec3 FSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CookTorranceSpecular(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float D = DGGX(NdotH, roughness);
    float G = GSmith(NdotV, NdotL, roughness);
    vec3 F = FSchlick(F0, VdotH);

    vec3 numerator = D * G * F;
    float denom = 4.0 * NdotV * NdotL + 1e-5;
    return numerator / denom;
}

vec3 DiffuseLambert(vec3 albedo) {
    return albedo / PI;
}

vec3 evaluateLight(
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 radiance,
    float ao
) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    // Fresnel F0
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance
    float D = DGGX(NdotH, roughness);
    float G = GSmith(NdotV, NdotL, roughness);
    vec3 F = FSchlick(F0, VdotH);

    vec3 specNumer = D * G * F;
    float denom = max(4.0 * NdotV * NdotL, 1e-6);
    vec3 spec = specNumer / denom;

    // Energy-conserving diffuse
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * (albedo / PI);

    // final contribution from this light
    return (diffuse + spec) * radiance * NdotL;
}

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float shadow = 1.0;
	float bias = 0.005;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(ShadowMaps[0], vec2(shadowCoord.st + offset)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
			shadow = 0.0;
		}
	}
	return shadow;

}

float filterPCF(vec4 sc, uint cascadeIndex)
{
	ivec2 texDim = textureSize(ShadowMaps[0], 0).xy;
	float scale = RendererSetting.pcfScale;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex);
			count++;
		}
	}
	return shadowFactor / count;
}

float ComputeCSM(vec3 worldPos, mat4 viewMatrix, vec3 N) {
    uint cascadeIndex = 0;
    vec4 viewPos = viewMatrix * vec4(worldPos, 1.0);
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(viewPos.z < u_Shadow.cascadeSplits[i]) {	
			cascadeIndex = i + 1;
		}
	}

    cascadeIndex = 0;
	vec4 shadowCoord = (biasMat * u_Shadow.lightSpaceMatrices[cascadeIndex]) * vec4(worldPos, 1.0);	

	float shadow = 0.0;
    uint enablePCF = RendererSetting.pcf;
	if (enablePCF == 1) {
		shadow = filterPCF(shadowCoord / shadowCoord.w, cascadeIndex);
	} else {
		shadow = textureProj(shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex);
	}

    return shadow;
}

float ComputeShadow(vec3 worldPos, mat4 viewMatrix, vec3 N) {
	vec4 shadowCoord = (biasMat * u_Shadow.lightSpaceMatrices[0] * mat4(1.0)) * vec4(worldPos, 1.0);	
    uint enablePCF = RendererSetting.pcf;
	if (enablePCF == 1) {
		return filterPCF(shadowCoord / shadowCoord.w, 0);
	} else {
		return textureProj(shadowCoord / shadowCoord.w, vec2(0.0), 0);
	}
}

void main() {
    MeshDraw draw = MeshDrawsBlended.meshDraws[drawId];

    MaterialData material = Materials.materials[draw.materialIndex];

    float alpha = 0.0;
    vec4 baseColor = material.baseColorFactor;
    alpha = baseColor.a;
    if (material.baseColorTextureIndex != -1) {
        baseColor *= texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], UV);
        alpha *= texture(GlobalTextures[nonuniformEXT(material.baseColorTextureIndex)], UV).a;
    }

    vec4 Albedo = vec4(baseColor.xyz, 1.0);

    vec4 FragPos = vec4(Position, 1.0);
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

    vec4 Emissive = vec4(material.emissiveFactor, 1.0);
    if (material.emissiveTextureIndex != -1) {
        Emissive *= texture(GlobalTextures[nonuniformEXT(material.emissiveTextureIndex)], UV);
    }
    Emissive *= material.emissiveStrength;

    float occlusion = 1.0;
    if (material.occlusionTextureIndex != -1) {
        occlusion = texture(GlobalTextures[nonuniformEXT(material.occlusionTextureIndex)], UV).r;
    }

    //float alpha = texture(sampler_Albedo, uv).a;

    vec3 fragPosView = vec3(MVP.View * FragPos);
    vec3 V = normalize(MVP.CamPos - FragPos.xyz);
    vec3 accum = vec3(0.0);

    uint clusterIdx = computeClusterIndex(fragPosView);

    // Fetch light indices
    uvec2 offsetCount = ClusterLightOffsetList.lightOffsets[clusterIdx];
    uint offset = offsetCount.x;
    uint count = offsetCount.y;

    vec3 LightDir = normalize(-sunLightDir);
    for (uint i = 0; i < count; ++i) {
        uint lightIdx = LightIndices.lightIndexList[offset + i];
        vec3 lightPos = LightsList.lights[lightIdx].position.xyz;
        float radius = LightsList.lights[lightIdx].position.w;
        vec3 lightColor = LightsList.lights[lightIdx].color.rgb;
        float intensity = LightsList.lights[lightIdx].color.w;

        vec3 toLight = lightPos - FragPos.xyz;
        float dist = length(toLight);
        vec3 L = normalize(toLight);

        float att = clamp(1.0 - dist/radius, 0.0, 1.0);
        vec3 radiance = lightColor * intensity * att;

        accum += evaluateLight(Albedo.xyz, metallic, roughness, Ns, V, L, radiance, occlusion);
    }

    bool cascadedShadows = RendererSetting.csm == 1;

    float shadow = 0.0f;
    if(cascadedShadows){
        //shadow = ComputeShadow(FragPos, MVP.View, N);
    }
    else {
        shadow = ComputeShadow(FragPos.xyz, MVP.View, Ns);
    }
    
    // Sun light
    vec3 sunColor = vec3(1.0, 0.95, 0.9);
    float sunIntensity = 20.0;
    vec3 radiance = sunColor * sunIntensity;
    accum +=  shadow * evaluateLight(Albedo.xyz, metallic, roughness, Ns, V, LightDir, radiance, occlusion);

    vec3 ambient = vec3(0.03);
    ambient = ambient * Albedo.xyz * occlusion;
    vec3 finalColor = ambient + accum + Emissive.xyz;

    finalColor = finalColor / (finalColor + vec3(1.0));
    //finalColor = pow(finalColor, vec3(1.0/2.2)); // gamma

    outColor = vec4(finalColor, alpha);
}
