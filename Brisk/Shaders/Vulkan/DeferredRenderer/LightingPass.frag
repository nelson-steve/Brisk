#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

// G-Buffer textures
layout(set = 2, binding = 0) uniform sampler2D gPosition;
layout(set = 2, binding = 1) uniform sampler2D gNormal;
layout(set = 2, binding = 2) uniform sampler2D gAlbedo;
layout(set = 2, binding = 3) uniform sampler2D gMaterial; // metallic roughness occlusion
layout(set = 2, binding = 4) uniform sampler2D gEmissive;

void main() {
    vec3 pos = texture(gPosition, uv).rgb;
    vec3 norm = normalize(texture(gNormal, uv).rgb);
    vec3 albedo = texture(gAlbedo, uv).rgb;
    vec3 emissive = texture(gEmissive, uv).rgb;

    vec4 matData = texture(gMaterial, uv);
    float metallic = matData.r;
    float roughness = matData.g;
    float occlusion = matData.b;
    bool unlit = (matData.a > 0.5);

    if (unlit) {
        outColor = vec4(albedo + emissive, 1.0);
        return;
    }

    // Basic lighting
    vec3 lightPos = vec3(2.0, 2.0, 2.0);
    vec3 lightColor = vec3(1.0);

    vec3 lightDir = normalize(lightPos - pos);
    vec3 viewPos = vec3(0.0, 0.0, 10.0);
    vec3 viewDir = normalize(viewPos - pos);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong simplified)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), mix(1.0, 128.0, 1.0 - roughness));
    vec3 specular = spec * lightColor * (1.0 - roughness);

    // Fresnel base reflectance (F0)
    vec3 dielectricF0 = vec3(0.04);
    vec3 F0 = mix(dielectricF0, albedo, metallic); // metals use albedo as F0

    // Diffuse is zero for metals
    vec3 kD = vec3(1.0 - metallic);
    float NdotV = max(dot(norm, viewDir), 0.0);
    vec3 fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
    vec3 color = kD * albedo * diffuse + fresnel * spec;

    color = pow(color, vec3(1.0 / 2.2)); // Gamma correction

    // Apply ambient occlusion
    color *= occlusion;

    // Add emissive
    color += emissive;

    outColor = vec4(albedo, 1.0);
}
