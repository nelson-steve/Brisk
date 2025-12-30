struct RadiancePayload {
    vec3 radiance;     // accumulated light
    vec3 throughput;   // energy carried
    vec3 nextOrigin;   // where to spawn next ray
    vec3 nextDir;      // direction of next ray
    uint rngState;
    bool done;         // terminate path?
};