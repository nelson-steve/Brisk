#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 18) uniform sampler2D Glow;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    int horizontal = 0;

    vec2 texelSize = 1.0 / textureSize(Glow, 0);
    vec3 result = texture(Glow, fragUV).rgb * weights[0];

    for (int i = 1; i < 5; ++i) {
        vec2 offset = texelSize * float(i) * vec2(horizontal, 1 - horizontal);
        result += texture(Glow, fragUV + offset).rgb * weights[i];
        result += texture(Glow, fragUV - offset).rgb * weights[i];
    }

    outColor = vec4(result, 1.0);
}
