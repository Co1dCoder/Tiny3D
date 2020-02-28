#include "shader/util.glsl"

layout(bindless_sampler) uniform sampler2D texBlds[MAX_TEX];
uniform vec3 uNormal;

in vec2 vTexcoord;
flat in vec4 vTexid;

layout (location = 0) out vec4 FragTex;
layout (location = 1) out vec4 FragMat;
layout (location = 2) out vec4 FragNormalGrass;
layout (location = 3) out vec4 FragRoughMetal;

void main() {
	vec4 textureColor = texture2D(texBlds[int(vTexid.x)], vTexcoord.xy);
	if(textureColor.a < 0.4) discard;
	
	FragTex = textureColor;
	FragMat = BoardMat;
	FragNormalGrass = vec4(uNormal, 0.0);
	FragRoughMetal = BoardRM;
}