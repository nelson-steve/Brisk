#version 450

layout (location = 0) in vec2 fragUV;
layout (location = 0) out float FragColor;

layout (set = 0, binding = 0) uniform sampler2D gPosition;
layout (set = 0, binding = 1) uniform sampler2D gNormal;
layout (set = 0, binding = 2) uniform sampler2D texNoise;

layout (set = 0, binding = 3) uniform UBO {
    mat4 projection;
    mat4 view;
    vec2 screenSize;
    float radius;
    float bias;
} ubo;

layout (set = 0, binding = 4) uniform SampleKernel {
    vec3 samples[64];
} kernel;

const int kernelSize = 64;

void main() {
    const vec2 noiseScale = vec2(ubo.screenSize.x / 4.0, ubo.screenSize.y / 4.0);

    vec3 fragPos = texture(gPosition, fragUV).xyz;
    vec3 normal = normalize(texture(gNormal, fragUV).xyz);
    vec3 randomVec = normalize(texture(texNoise, fragUV * noiseScale).xyz);

    // Construct TBN matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // Sample in tangent space
        vec3 sampleVec = TBN * kernel.samples[i];
        sampleVec = fragPos + sampleVec * ubo.radius;

        // Project sample position (view-space) to screen-space
        vec4 offset = ubo.projection * vec4(sampleVec, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = texture(gPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, ubo.radius / abs(fragPos.z - sampleDepth));
        if ((sampleDepth + ubo.bias) < sampleVec.z) {
            occlusion += rangeCheck;
        }
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}
