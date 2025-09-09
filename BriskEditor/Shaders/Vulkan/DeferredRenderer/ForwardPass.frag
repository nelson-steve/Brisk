#version 450

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D albedoMap;
layout(set = 0, binding = 2) uniform sampler2D lightingBuffer;

layout(set = 0, binding = 3) uniform Camera {
    vec3 camPos;
} camera;

layout(set = 0, binding = 4) uniform Light {
    vec3 lightPos;
    vec3 lightColor;
} light;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(light.lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 albedo = texture(albedoMap, fragUV).rgb;
    vec3 lighting = diff * light.lightColor;

    vec4 lightingPassColor = texture(lightingBuffer, fragUV);
    outColor = vec4(lightingPassColor.rgb + albedo * lighting, 1.0);
}
