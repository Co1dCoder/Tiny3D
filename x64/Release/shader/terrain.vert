#version 330

uniform mat4 viewProjectMatrix;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 texcoord;
layout (location = 3) in vec3 color;

out vec4 vTexcoord;
flat out vec3 vColor;
out vec3 vNormal;
out float worldHeight;

void main() {
	vColor = vec3(0.1, 1.8, 1.0) * color * 0.005;
	
	vec4 worldVertex = vec4(vertex, 1.0);
	vNormal = normal;

	worldHeight = worldVertex.y;
	
	vTexcoord = texcoord; 
	gl_Position = viewProjectMatrix * worldVertex;
}