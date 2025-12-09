#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive: require

#include "RTShared.glsl"

layout(set = 0, binding = 11) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 13) uniform CameraUBO {
    mat4 viewInv;
    mat4 projInv;
    vec3 lightPos;
    vec3 camPos;
    vec2 dimension; // width, height
    vec2 _pad;
} camera;

layout(location = 0) rayPayloadInEXT RadiancePayload radiancePayload;
layout(location = 2) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

void main() {
    radiancePayload.color = vec3(1.0, 1.0, 1.0);

	vec3 lightVector = normalize(-camera.lightPos);
    vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	float tmin = 0.001;
	float tmax = 10000.0;

    shadowPayload.occluded = 1u;

    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
    if(shadowPayload.occluded == 1u){
        radiancePayload.color = vec3(0.2, 0.2, 0.2);
    }
}