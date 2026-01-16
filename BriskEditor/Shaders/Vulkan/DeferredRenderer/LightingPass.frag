#version 450

#define PI 3.14159265359

#include "../config.hpp"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;

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

// G-buffer textures
layout(set = 2, binding = 0) uniform sampler2D sampler_Position;
layout(set = 2, binding = 1) uniform sampler2D sampler_Normal;
layout(set = 2, binding = 2) uniform sampler2D sampler_Albedo;
layout(set = 2, binding = 3) uniform sampler2D sampler_Material; // metal/rough/occlusion
layout(set = 2, binding = 4) uniform sampler2D sampler_Emissive;

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

layout(std430, set = 3,  binding = 3) buffer ClusterLightCountsListBuffer {
    uint lightCounts[];
} ClusterLightCountsList;

layout(std140, set = 0, binding = 6) uniform ShadowData {
    mat4 lightSpaceMatrices[SHADOW_MAP_CASCADE_COUNT];
    vec4 cascadeSplits;
} u_Shadow;

layout(set = 0, binding = 7) uniform sampler2D ShadowMaps[SHADOW_MAP_CASCADE_COUNT];

layout(push_constant) uniform PushConstants {
    vec3 sunLightDir;
};

uint computeClusterIndex(vec3 fragPosView) {
    vec2 fragCoord = gl_FragCoord.xy;
    // TODO: Get viewport size
    uvec2 screenSize = uvec2(textureSize(sampler_Position, 0));

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
    vec3 albedo = texture(sampler_Albedo, uv).rgb;
    float alpha = texture(sampler_Albedo, uv).a;
    vec4 mat = texture(sampler_Material, uv); // Occlusion Roughness Metallic
    float ao = mat.r;
    float roughness = mat.g;
    float metallic = mat.b;
    vec3 N = normalize(texture(sampler_Normal, uv).xyz * 2.0 - 1.0);
    vec3 emissive = texture(sampler_Emissive, uv).rgb;
    vec3 fragPos = texture(sampler_Position, uv).rgb;

    vec3 fragPosView = vec3(MVP.View * vec4(fragPos, 1.0));
    vec3 V = normalize(MVP.CamPos - fragPos);
    vec3 accum = vec3(0.0);

    uint clusterIdx = computeClusterIndex(fragPosView);

    // Fetch light indices
    uint count = ClusterLightCountsList.lightCounts[clusterIdx];
    uint offset = MAX_LIGHTS_PER_CLUSTER * clusterIdx;
    vec3 LightDir = normalize(-sunLightDir);
    for (uint i = 0; i < count; ++i) {
        uint lightIdx = LightIndices.lightIndexList[offset + i];
        vec3 lightPos = LightsList.lights[lightIdx].position.xyz;
        float radius = LightsList.lights[lightIdx].position.w;
        vec3 lightColor = LightsList.lights[lightIdx].color.rgb;
        float intensity = LightsList.lights[lightIdx].color.w;

        vec3 toLight = lightPos - fragPos;
        float dist = length(toLight);
        vec3 L = normalize(toLight);

        float att = clamp(1.0 - dist/radius, 0.0, 1.0);
        vec3 radiance = lightColor * intensity * att;

        accum += evaluateLight(albedo, metallic, roughness, N, V, L, radiance, ao);
    }

    bool cascadedShadows = RendererSetting.csm == 1;

    float shadow = 0.0f;
    if(cascadedShadows){
        //shadow = ComputeShadow(fragPos, MVP.View, N);
    }
    else {
        shadow = ComputeShadow(fragPos, MVP.View, N);
    }
    
    // Sun light
    vec3 sunColor = vec3(1.0, 0.95, 0.9);
    float sunIntensity = 20.0;
    vec3 radiance = sunColor * sunIntensity;
    accum += shadow * evaluateLight(albedo, metallic, roughness, N, V, LightDir, radiance, ao);

    vec3 ambient = vec3(0.03);
    ambient = ambient * albedo * ao;
    vec3 finalColor = ambient + accum + emissive;

    finalColor = finalColor / (finalColor + vec3(1.0));
    //finalColor = pow(finalColor, vec3(1.0/2.2)); // gamma

    outColor = vec4(finalColor, 1.0);

    //
    float brightness = max(max(finalColor.r, finalColor.g), finalColor.b);

    float soft = brightness - RendererSetting.threshold + RendererSetting.knee;
    soft = clamp(soft / (2.0 * RendererSetting.knee), 0.0, 1.0);

    float contribution = max(brightness - RendererSetting.threshold, 0.0);
    contribution += soft * soft * RendererSetting.knee;

    outBrightColor = vec4(outColor.xyz * contribution / max(brightness, 0.0001), 1.0);
}
