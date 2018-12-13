#version 330

uniform mat4 viewProjectMatrix;
uniform mat4 uModelMatrix;
uniform mat3x4 boneMats[100];

layout (location = 0) in vec3 vertex;
layout (location = 4) in vec4 boneids;
layout (location = 5) in vec4 weights;

mat4 convertMat(mat3x4 srcMat) {
	mat4x3 transMat = transpose(srcMat);
	return mat4(transMat[0], 0.0, 
				transMat[1], 0.0, 
				transMat[2], 0.0, 
				transMat[3], 1.0);
}

void main() {	
	mat4 boneMat = convertMat(boneMats[int(boneids.x)]) * weights.x;
    boneMat += convertMat(boneMats[int(boneids.y)]) * weights.y;
    boneMat += convertMat(boneMats[int(boneids.z)]) * weights.z;
    boneMat += convertMat(boneMats[int(boneids.w)]) * weights.w;
    
    vec4 position = boneMat * vec4(vertex, 1.0);
	
	vec4 modelPosition = uModelMatrix * position;
	gl_Position = viewProjectMatrix * modelPosition;
}
