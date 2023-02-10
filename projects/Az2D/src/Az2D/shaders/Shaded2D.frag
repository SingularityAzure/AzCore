#version 450
#extension GL_ARB_separate_shader_objects : enable
// This extension allows us to use std430 for our uniform buffer
// Without it, the arrays of scalars would have a stride of 16 bytes
#extension GL_EXT_scalar_block_layout : enable
// So we can have a smaller uniform buffer
#extension GL_EXT_shader_8bit_storage : enable

layout (location = 0) in vec2 inTexCoord;
layout (location = 1) in vec3 inScreenPos;
layout (location = 2) in mat2 inTransform;

layout (location = 0) out vec4 outColor;

const int MAX_LIGHTS = 256;
const int MAX_LIGHTS_PER_BIN = 16;
const int LIGHT_BIN_COUNT_X = 32;
const int LIGHT_BIN_COUNT_Y = 18;

struct Light {
	// pixel-space position
	vec3 position;
	vec3 color;
	// How much light reaches the surface at a 90-degree angle of incidence in the range of 0.0 to 1.0
	float attenuation;
	// A normalized vector
	vec3 direction;
	// angular falloff in cos(radians) where < min is 100% brightness, between min and max blends, and > max is 0% brightness
	float angleMin;
	float angleMax;
	// distance-based falloff in pixel-space where < min is 100% brightness, between min and max blends, and > max is 0% brightness
	float distMin;
	float distMax;
};

struct LightBin {
	uint8_t lightIndices[MAX_LIGHTS_PER_BIN];
};

layout(std430, set=0, binding=0) uniform UniformBuffer {
	vec2 screenSize;
	vec3 ambientLight;
	LightBin lightBins[LIGHT_BIN_COUNT_X*LIGHT_BIN_COUNT_Y];
	Light lights[MAX_LIGHTS];
} ub;

layout(set=0, binding=1) uniform sampler2D texSampler[1];

layout(push_constant) uniform pushConstants {
	layout(offset = 48) vec4 color;
	layout(offset = 64) int texAlbedo;
	layout(offset = 68) int texNormal;
	layout(offset = 72) float normalAttenuation;
} pc;

float map(float a, float min1, float max1, float min2, float max2) {
	return (a - min1) / (max1 - min1) * (max2 - min2) + min2;
}

float mapClamped(float a, float min1, float max1, float min2, float max2) {
	return clamp(map(a, min1, max1, min2, max2), min2, max2);
}

float square(float a) {
	return a * a;
}
float smoothout(float a) {
	return smoothstep(0.0, 1.0, a);
}

vec3 DoLighting(vec3 normal) {
	int binX = clamp(int(inScreenPos.x * float(LIGHT_BIN_COUNT_X) / ub.screenSize.x), 0, LIGHT_BIN_COUNT_X-1);
	int binY = clamp(int(inScreenPos.y * float(LIGHT_BIN_COUNT_Y) / ub.screenSize.y), 0, LIGHT_BIN_COUNT_Y-1);
	int bindex = binY * LIGHT_BIN_COUNT_X + binX;
	vec3 lighting = ub.ambientLight;
	for (int i = 0; i < MAX_LIGHTS_PER_BIN; i++) {
		uint lightIndex = uint(ub.lightBins[bindex].lightIndices[i]);
		Light light = ub.lights[lightIndex];
		vec3 dPos = inScreenPos - light.position;
		float dist = length(dPos);
		dPos /= dist;
		float factor = smoothout(square(mapClamped(dist, light.distMax, light.distMin, 0.0, 1.0)));
		float angle = acos(dot(light.direction, dPos));
		factor *= smoothout(square(mapClamped(angle, light.angleMax, light.angleMin, 0.0, 1.0)));
		float incidence = square(clamp(dot(normal, -dPos), 0.0, 1.0));
		factor *= mix(light.attenuation, 1.0, incidence);
		lighting += light.color * factor;
	}
	return lighting;
}

void main() {
	vec3 normal = texture(texSampler[pc.texNormal], inTexCoord).rgb * 2.0 - vec3(1.0);
	normal.xy = inTransform * normal.xy;
	normal = mix(vec3(0.0, 0.0, 1.0), normal, pc.normalAttenuation);
	// Conditional normalize
	float len = length(normal);
	if (len > 0.1) normal /= len;

	outColor = texture(texSampler[pc.texAlbedo], inTexCoord) * pc.color;
	outColor.rgb *= DoLighting(normal);
}
