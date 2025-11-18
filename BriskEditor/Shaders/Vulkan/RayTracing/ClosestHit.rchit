#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;

hitAttributeEXT vec2 attribs;

void main()
{
    hitValue = vec3(0.0);
}
