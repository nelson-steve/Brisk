#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"

layout(location = 2) rayPayloadInEXT ShadowPayload shadowPayload;

void main()
{
    shadowPayload.occluded = 0u;
}