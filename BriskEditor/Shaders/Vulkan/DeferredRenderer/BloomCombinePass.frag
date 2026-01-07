#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 17) uniform sampler2D scene;
layout (set = 0, binding = 20) uniform sampler2D bloom;
layout(std140, set = 0, binding = 21) uniform BloomSettingBuffer {
    float knee;
    float threshold;
    float intensity;
    int _pad;
} BloomSetting;

void main() {
    vec3 base = texture(scene, fragUV).rgb;
    vec3 bloomColor = texture(bloom, fragUV).rgb;
    outColor = vec4(base + bloomColor * BloomSetting.intensity, 1.0);
}
