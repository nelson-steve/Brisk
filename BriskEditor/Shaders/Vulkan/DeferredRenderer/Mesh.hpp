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

struct MeshDraw
{
    vec3 position;
    float scale;
    vec4 orientation;

    uint meshIndex;
    uint meshletVisibilityOffset;
    uint postPass;
    uint materialIndex;
};