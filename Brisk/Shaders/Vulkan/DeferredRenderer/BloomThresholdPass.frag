#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D hdrScene;

void main() {
    vec3 color = texture(hdrScene, fragUV).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // luminance
    if (brightness > 1.0) // threshold
        outColor = vec4(color, 1.0);
    else
        outColor = vec4(0.0);
}
