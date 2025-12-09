struct RadiancePayload
{
    vec3 color;
    float hitT;
};

struct ShadowPayload
{
    uint occluded;
};