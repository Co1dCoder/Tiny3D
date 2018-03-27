#version 330
#extension GL_EXT_gpu_shader4 : enable 

in vec2 projDepth;

layout (location = 0) out vec4 FragColor;

void main() {
	float depth = projDepth.x / projDepth.y;
	depth = depth * 0.5 + 0.5;
	FragColor = vec4(depth, 0.0, 0.0, 1.0);
}