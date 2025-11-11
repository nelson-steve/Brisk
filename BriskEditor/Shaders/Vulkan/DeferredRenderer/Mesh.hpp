struct Vertex {
    float vx, vy, vz;
    float nx, ny, nz;
    float tx, ty;
};

struct Meshlet
{
	vec3 center;
	float radius;
	int8_t cone_axis[3];
	int8_t cone_cutoff;

	uint dataOffset;
	uint baseVertex;
	uint8_t vertexCount;
	uint8_t triangleCount;
	uint8_t shortRefs;
};

struct Transform {
    vec3 position;
    float scale;
    vec4 orientation;
};

struct MeshDraw
{
    uint transformIndex;
    uint meshIndex;
    uint materialIndex;
    uint meshletCount;
    uint meshletOffset;

    uint groupcount[3];
};

vec3 rotateQuat(vec3 v, vec4 q)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}