#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D image;
layout(push_constant) uniform PushConstants {
    int horizontal;
};

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, fragUV).rgb * weights[0];

    for (int i = 1; i < 5; ++i) {
        vec2 offset = texelSize * float(i) * vec2(blur.horizontal, 1 - blur.horizontal);
        result += texture(image, fragUV + offset).rgb * weights[i];
        result += texture(image, fragUV - offset).rgb * weights[i];
    }

    outColor = vec4(result, 1.0);
}
