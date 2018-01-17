#version 330
#extension GL_EXT_gpu_shader4 : enable 

uniform sampler2DArray texture;
uniform vec3 light;
uniform int useShadow;
uniform float level1, level2;
uniform sampler2D depthBufferNear, depthBufferMid, depthBufferFar;

in vec2 vTexcoord;
flat in vec3 vColor;
flat in vec4 vTexid;
in vec3 vNormal;
in vec4 worldPosition;
in vec4 projPosition;
in vec4 viewPosition;
in vec4 lightNearPosition,lightMidPosition,lightFarPosition;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragDepth;
layout (location = 2) out vec4 FragNormal;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float genPCF(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	float shadowFactor = 1.0;
	for (int i=0; i < 4; i++) {
		int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
		float factor = texture2D(shadowMap, shadowCoord.xy + poissonDisk[index]/700.0).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
		shadowFactor -= 0.25 * (1.0 - factor);
	}
	return shadowFactor;
}

float genVSM(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	vec2 moments = texture2D(shadowMap,shadowCoord.xy).rg;
	float depth = shadowCoord.z - bias;
	if (depth <= moments.x)
		return 1.0 ;
			
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance,0.00002);
	
	float d = depth - moments.x;
	float pMax = variance / (variance + d * d);
	return pMax;
}

float genShadow(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	float shadowFactor = texture2D(shadowMap, shadowCoord.xy).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
	return shadowFactor;
}

float genShadowFactor(float bias) {
	float depthView = -viewPosition.z / viewPosition.w;
	
	if(depthView <= level1) {
		vec3 lightPosition = lightNearPosition.xyz / lightNearPosition.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.0005;
		return genPCF(depthBufferNear, shadowCoord, bs);
	} else if(depthView <= level2) {
		vec3 lightPosition = lightMidPosition.xyz / lightMidPosition.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.00005;
		return genShadow(depthBufferMid, shadowCoord, bs);
	} else {
		vec3 lightPosition = lightFarPosition.xyz / lightFarPosition.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.000005;
		return genShadow(depthBufferFar, shadowCoord, bs);
	}

	return 1.0;
}

void main() {
	vec4 textureColor = vec4(1.0);

	vec4 textureColor1 = vec4(1.0);
	vec3 texcoord1 = vec3(vTexcoord, vTexid.x);
	if(texcoord1.p >= 0.0) 
		textureColor1 = texture2DArray(texture, texcoord1);
	
	vec4 textureColor2 = vec4(1.0);
	vec3 texcoord2 = vec3(vTexcoord, vTexid.y);
	if(texcoord2.p >= 0.0) 
		textureColor2 = texture2DArray(texture, texcoord2);
		
	float height = worldPosition.y;
	float blendPer = smoothstep(100.0, 200.0, height);
	textureColor = (1.0 - blendPer) * textureColor1 + blendPer * textureColor2;


	vec3 normal = normalize(vNormal);
		
	vec3 reverseLight = normalize(-light);
	float ndotl = dot(reverseLight, normal);
	float bias = tan(acos(abs(ndotl)));
		
	float ambientFactor = 1.0; float diffuseFactor = 1.0;
	vec3 ambientColor = vColor.xxx * ambientFactor;
	vec3 diffuseColor = vColor.yyy * diffuseFactor;
		
	diffuseColor *= max(ndotl, 0.0);

	float shadowFactor = 1.0;
	if(useShadow > 0)
		shadowFactor = genShadowFactor(bias);
		
	FragColor.rgb = textureColor.rgb * (ambientColor + shadowFactor * diffuseColor);
	FragColor.a = 1.0;

	float depth = projPosition.z / projPosition.w;
	FragDepth = vec4(depth, depth, depth, depth);
	vec3 outNormal = normal * 0.5 + 0.5;
	FragNormal = vec4(outNormal, 0.0); 
}