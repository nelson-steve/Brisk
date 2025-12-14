uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float Random(inout uint seed)
{
    seed = Hash(seed);
    return float(seed) / float(0xffffffffu);
}

vec3 CosineHemisphere(vec2 xi)
{
    float r = sqrt(xi.x);
    float theta = 2.0 * 3.14159265 * xi.y;

    return vec3(
        r * cos(theta),
        r * sin(theta),
        sqrt(max(0.0, 1.0 - xi.x))
    );
}

mat3 BuildTBN(vec3 N)
{
    vec3 T = normalize(abs(N.z) < 0.999 ? cross(N, vec3(0,0,1)) : cross(N, vec3(0,1,0)));
    vec3 B = cross(N, T);
    return mat3(T, B, N);
}
