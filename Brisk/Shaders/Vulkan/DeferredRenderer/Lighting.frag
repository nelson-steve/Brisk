#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

//layout(set = 1, binding = 0) uniform Light {
//    vec3 lightPos;
//    vec3 lightColor;
//    vec3 viewPos;
//} light;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput gPosition;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput gNormal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput gAlbedo;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput gDepth;

void main() {
    //vec3 pos = subpassLoad(gPosition, uv).rgb;
    //vec3 norm = normalize(texture(gNormal, uv).rgb);
    //vec3 albedo = texture(gAlbedo, uv).rgb;

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
    //outColor = vec4(norm, 1.0);

    vec4 albedo = subpassLoad(gAlbedo);
    vec3 normal = subpassLoad(gNormal).xyz;
    vec4 pos = subpassLoad(gPosition);

    //outColor = vec4(1.0, 1.0, 0.0, 1.0);
    outColor = vec4(pos);
}
