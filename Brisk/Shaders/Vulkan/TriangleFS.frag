#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedo;
layout(binding = 2) uniform sampler2D roughness;
layout(binding = 3) uniform sampler2D normal;
layout(binding = 4) uniform sampler2D emissive;
layout(binding = 5) uniform sampler2D metallic;

//layout(location = 0) in vec3 inUV0;
//layout(location = 1) in vec3 inUV1;

void main() {
    outColor = vec4(fragColor, 1.0);
}