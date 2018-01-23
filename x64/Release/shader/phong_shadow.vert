#version 330

uniform mat4 viewProjectMatrix;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 texcoord;
layout (location = 2) in mat3x4 modelMatrix;

out vec2 vTexcoord;
flat out float vTexid;
out vec4 projPosition;

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
	vTexcoord = texcoord.xy;
	vTexid = texcoord.z; 
	vec4 worldVertex = convertMat(modelMatrix) * vec4(vertex, 1.0);
	gl_Position = viewProjectMatrix * worldVertex;
	projPosition = gl_Position;
}