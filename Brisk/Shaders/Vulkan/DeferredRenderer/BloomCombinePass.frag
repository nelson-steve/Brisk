#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D scene;
layout (set = 0, binding = 1) uniform sampler2D bloom;
layout (set = 0, binding = 2) uniform BloomSettings {
    float intensity;
} settings;

void main() {
    vec3 base = texture(scene, fragUV).rgb;
    vec3 bloomColor = texture(bloom, fragUV).rgb;
    outColor = vec4(base + bloomColor * settings.intensity, 1.0);
}
