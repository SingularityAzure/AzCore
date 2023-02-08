#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform sampler2D texSampler[1];

layout(push_constant) uniform pushConstants {
	layout(offset = 32) vec4 color;
	layout(offset = 48) int texId;
	layout(offset = 52) int texNormal;
	layout(offset = 56) float normalAttenuation;
	layout(offset = 60) float edge;
	layout(offset = 64) float bounds;
} pc;

void main() {
	outColor = pc.color;
	outColor *= smoothstep(pc.bounds-pc.edge, pc.bounds+pc.edge, texture(texSampler[pc.texId],texCoord).r);
	outColor.rgb *= pc.color.a;
}
