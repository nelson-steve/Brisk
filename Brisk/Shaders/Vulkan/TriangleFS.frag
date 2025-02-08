#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

//layout(binding = 1) uniform sampler2D albedo;
//layout(binding = 2) uniform sampler2D roughness;
//layout(binding = 3) uniform sampler2D normal;
//layout(binding = 4) uniform sampler2D emissive;
//layout(binding = 5) uniform sampler2D metallic;

#extension GL_EXT_nonuniform_qualifier : enable

layout ( set = 1, binding = 10 ) uniform sampler2D global_textures[];
// Alias textures to use the same binding point, as bindless texture is shared
// between all kind of textures: 1d, 2d, 3d.
layout ( set = 1, binding = 10 ) uniform sampler3D global_textures_3d[];

layout(location = 1) in vec2 inUV0;
layout(location = 2) in vec2 inUV1;

void main() {
    int textureIndex = 0;
    vec4 color = texture(global_textures[nonuniformEXT(textureIndex)], inUV0);
    outColor = vec4(color.xyz, 1.0);
}