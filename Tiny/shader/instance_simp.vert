#version 450

uniform mat4 viewProjectMatrix;
uniform float shadowPass;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 texcoord;
layout (location = 3) in vec2 texid;
layout (location = 4) in vec3 color;
layout (location = 5) in vec3 tangent;
layout (location = 6) in vec4 modelTrans;

out vec2 vTexcoord;
flat out vec4 vTexid;
flat out vec3 vColor;
out vec3 vNormal;
out mat3 vTBN;

#define COLOR_SCALE vec3(0.003, 0.006, 0.005)

mat3 GetTBN(vec3 normal, vec3 tangent) {
	vec3 bitangent = cross(normal, tangent);
	return mat3(tangent, bitangent, normal);
}

void main() {
	vec4 worldVertex = vec4(modelTrans.w * vertex + modelTrans.xyz, 1.0);
	if(shadowPass < 0.5) {
		vColor = COLOR_SCALE * color;
		vNormal = normal;
		vTBN = GetTBN(normalize(normal), normalize(tangent));
	}
	vTexcoord = texcoord.xy;
	vTexid = vec4(texcoord.zw, texid);
	gl_Position = viewProjectMatrix * worldVertex;
}