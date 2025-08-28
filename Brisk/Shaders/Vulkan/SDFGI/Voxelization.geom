#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec3 inNormal[];
layout(location = 2) in vec2 inUV[];

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec2 gUV;

void main() {
    for(int i = 0; i < 3; ++i) {
        gPosition = inPosition[i];
        gNormal = inNormal[i];
        gUV = inUV[i];
        gl_Position = vec4(inPosition[i], 1.0);
        EmitVertex();
    }
    EmitVertex();
}

