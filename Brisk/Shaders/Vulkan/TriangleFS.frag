#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
	uint albedoIndex;
	uint metallicRoughnessIndex;
	uint normalIndex;
	uint emissiveIndex;
	uint occlusionIndex;
} pushConstants;

#extension GL_EXT_nonuniform_qualifier : enable

layout ( set = 1, binding = 10 ) uniform sampler2D global_textures[];
// Alias textures to use the same binding point, as bindless texture is shared
// between all kind of textures: 1d, 2d, 3d.
layout ( set = 1, binding = 10 ) uniform sampler3D global_textures_3d[];

layout(location = 1) in vec2 inUV0;
layout(location = 2) in vec2 inUV1;

void main() {
    int textureIndex = 0;
    vec4 color = texture(global_textures[nonuniformEXT(pushConstants.normalIndex)], inUV0);
    outColor = vec4(color.xyz, 1.0);
}