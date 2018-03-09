#version 330

uniform mat4 viewProjectMatrix;
uniform vec3 light;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;
layout (location = 3) in vec3 color;
layout (location = 4) in mat3x4 modelMatrix;

out vec2 vTexcoord;
flat out float vTexid;
out vec3 vColor;
out vec3 vNormal;
out vec4 projPosition;

mat4 convertMat(mat3x4 srcMat) {
	vec4 col1 = srcMat[0];
	vec4 col2 = srcMat[1];
	vec4 col3 = srcMat[2];
	vec4 row1 = vec4(col1.x, col2.x, col3.x, 0.0);
	vec4 row2 = vec4(col1.y, col2.y, col3.y, 0.0);
	vec4 row3 = vec4(col1.z, col2.z, col3.z, 0.0);
	vec4 row4 = vec4(col1.w, col2.w, col3.w, 1.0);
	return mat4(row1, row2, row3, row4);
}

void main() {
	mat4 matModel = convertMat(modelMatrix);
	vec4 worldVertex = matModel * vec4(vertex, 1.0);
	vec3 worldNormal = (matModel * vec4(normal, 0.0)).xyz;
	vNormal = worldNormal;

	vec3 matColor = color * 0.005;
	float ambientFactor = 0.5; float diffuseFactor = 1.0;
	vec3 ambientColor = matColor.xxx * ambientFactor;
	vec3 diffuseColor = matColor.yyy * diffuseFactor;
	vec3 invLight = normalize(-light);
	float ndotl = dot(invLight, normalize(worldNormal));
	diffuseColor *= max(ndotl, 0.0);
	vColor = ambientColor + diffuseColor;
	
	vTexcoord = texcoord.xy; 
	vTexid = texcoord.z;
	gl_Position = viewProjectMatrix * worldVertex;
	projPosition = gl_Position;
}