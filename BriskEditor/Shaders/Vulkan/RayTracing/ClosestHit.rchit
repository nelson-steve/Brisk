#version 460
#extension GL_EXT_ray_tracing : require

struct Payload {
    vec3 color;
};

layout(location = 0) rayPayloadInEXT Payload payload;

hitAttributeEXT vec2 attribs;

void main() {
    payload.color = vec3(1.0, 0.0, 0.0); // flat white for every hit
}
