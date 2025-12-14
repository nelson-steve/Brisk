struct RadiancePayload
{
    vec3 radiance;
    uint depth;
};

struct ShadowPayload
{
    uint occluded;
};