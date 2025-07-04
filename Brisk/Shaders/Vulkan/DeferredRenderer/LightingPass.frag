#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

// G-Buffer textures
layout(set = 2, binding = 0) uniform sampler2D gPosition;
layout(set = 2, binding = 1) uniform sampler2D gNormal;
layout(set = 2, binding = 2) uniform sampler2D gAlbedo;
layout(set = 2, binding = 3) uniform sampler2D gMaterial; // metallic roughness occlusion
layout(set = 2, binding = 4) uniform sampler2D gEmissive;

// Constants
const float PI = 3.14159265359;

// --- Helper Functions ---

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// --- Main ---

void main() {
    vec3 pos      = texture(gPosition, uv).rgb;
    vec3 N   = normalize(texture(gNormal, uv).rgb);
    vec3 albedo   = texture(gAlbedo, uv).rgb;
    float alpha   = texture(gAlbedo, uv).a;
    vec3 emissive = texture(gEmissive, uv).rgb;
    vec4 matData = texture(gMaterial, uv);

    float metallic  = matData.r;
    //float roughness = clamp(matData.g, 0.05, 1.0); // Avoid 0 roughness
    float roughness = max(matData.g, 0.001);
    float occlusion = matData.b;
    bool unlit = (matData.a > 0.5);

    //if (unlit) {
    //    outColor = vec4(albedo + emissive, 1.0);
    //    return;
    //}

    // Camera and light setup
    vec3 lightPos = vec3(0.0, 3.0, 0.0);
    vec3 lightColor = vec3(1.0);

    vec3 L = normalize(lightPos - pos);
    vec3 V = normalize(vec3(0.0, 0.0, 10.0) - pos);
    vec3 H = normalize(V + L);

    float distance = length(lightPos - pos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Fresnel reflectance at normal incidence
    //vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F0 = albedo;

    // BRDF components
    float  D = DistributionGGX(N, H, roughness);
    float  G = GeometrySmith(N, V, L, roughness);
    vec3   F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular
    vec3 numerator   = D * G * F;
    float denom      = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular    = numerator / denom;

    // kS = specular, kD = diffuse
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 diffuse = (albedo / PI);

    //vec3 radiance = lightColor; // No attenuation yet

    vec3 color = (kD * diffuse + specular) * radiance * NdotL;

    // Apply ambient occlusion
    //color *= occlusion;

    //vec3 ambient = vec3(0.03) * albedo * occlusion;
    //
    //color = color + ambient;

    // Add emissive
    color += emissive;

    // Tone mapping (simple Reinhard)
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, alpha);
}
