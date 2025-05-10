#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

//layout(set = 1, binding = 0) uniform Light {
//    vec3 lightPos;
//    vec3 lightColor;
//    vec3 viewPos;
//} light;

layout(set = 2, binding = 0) uniform sampler2D gPosition;
layout(set = 2, binding = 1) uniform sampler2D gNormal;
layout(set = 2, binding = 2) uniform sampler2D gAlbedo;

void main() {
    vec3 pos = texture(gPosition, uv).rgb;
    vec3 norm = normalize(texture(gNormal, uv).rgb);
    vec3 albedo = texture(gAlbedo, uv).rgb;

    //vec3 lightDir = normalize(light.lightPos - pos);
    //float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = diff * light.lightColor;
    //
    //vec3 viewDir = normalize(light.viewPos - pos);
    //vec3 reflectDir = reflect(-lightDir, norm);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    //vec3 specular = spec * light.lightColor;
    //
    //vec3 result = (diffuse + specular) * albedo;
    //outColor = vec4(result, 1.0);
    outColor = vec4(0.0, 1.0, 1.0, 1.0);
}
