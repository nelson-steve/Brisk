#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"

layout(location = 0) rayPayloadInEXT RadiancePayload radiancePayload;

void main() {
    vec3 sky = vec3(0.7, 0.9, 1.0);
    radiancePayload.radiance += radiancePayload.throughput * sky;
    radiancePayload.done = true;
}
