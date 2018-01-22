#version 330

uniform mat4 viewMatrix;
uniform mat4 projectMatrix;
uniform mat4 lightViewProjNear, lightViewProjMid, lightViewProjFar;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;
layout (location = 3) in vec3 color;
layout (location = 4) in mat3x4 modelMatrix;
layout (location = 7) in mat3 normalMatrix;

out vec2 vTexcoord;
flat out float vTexid;
flat out vec3 vColor;
out vec3 vNormal;
out vec4 projPosition;
out vec4 viewPosition;
out vec4 lightNearPosition,lightMidPosition,lightFarPosition;

mat4 convertMat(mat3x4 srcMat) {
	vec4 col1 = srcMat[0];
	vec4 col2 = srcMat[1];
	vec4 col3 = srcMat[2];
	vec4 row1 = vec4(col1.x, col2.x, col3.x, 0);
	vec4 row2 = vec4(col1.y, col2.y, col3.y, 0);
	vec4 row3 = vec4(col1.z, col2.z, col3.z, 0);
	vec4 row4 = vec4(col1.w, col2.w, col3.w, 1);
	return mat4(row1, row2, row3, row4);
}

void main() {
	vColor = color * 0.005;
	
	vec4 worldVertex = convertMat(modelMatrix) * vec4(vertex, 1.0);
	vNormal = normalMatrix * normal;
	
	vTexcoord = texcoord.xy; 
	vTexid = texcoord.z;
	viewPosition = viewMatrix * worldVertex;
	gl_Position = projectMatrix * viewPosition;
	projPosition = gl_Position;

	lightNearPosition = lightViewProjNear * worldVertex;
	lightMidPosition = lightViewProjMid * worldVertex;
	lightFarPosition = lightViewProjFar * worldVertex;
}