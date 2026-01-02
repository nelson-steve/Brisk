#version 450

layout(set = 0, binding = 17) uniform sampler2D LightingOutput;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

vec3 ACESFilm(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdr = texture(LightingOutput, uv).rgb;

    // Exposure
    float exposure = 1.0;
    hdr *= exposure;

    vec3 mapped = ACESFilm(hdr);

    // Gamma correction
    //mapped = pow(mapped, vec3(1.0 / 2.2));

    outColor = vec4(mapped, 1.0);
}