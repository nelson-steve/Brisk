#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

// G-Buffer textures
layout(set = 2, binding = 0) uniform sampler2D gPosition;
layout(set = 2, binding = 1) uniform sampler2D gNormal;
layout(set = 2, binding = 2) uniform sampler2D gAlbedo;
layout(set = 2, binding = 3) uniform sampler2D gMaterial;

void main() {
    // Fetch G-Buffer data
    vec3 pos = texture(gPosition, uv).rgb;
    vec3 norm = normalize(texture(gNormal, uv).rgb);
    vec3 albedo = texture(gAlbedo, uv).rgb;
    vec3 material = texture(gMaterial, uv).rgb; // You can use this if you store roughness, metalness, etc.

    // === Temporary light values ===
    vec3 lightPos = vec3(10.0, 10.0, 10.0);       // Arbitrary light position
    vec3 lightColor = vec3(1.0, 1.0, 1.0);        // White light
    vec3 viewPos = vec3(0.0, 0.0, 10.0);          // Arbitrary camera position

    // === Lighting Calculations ===
    vec3 lightDir = normalize(lightPos - pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - pos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    // Combine lighting with albedo
    vec3 result = (diffuse + specular) * albedo;

    outColor = vec4(result, 1.0);
}
