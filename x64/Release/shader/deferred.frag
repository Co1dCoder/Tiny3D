#version 330

uniform sampler2D texBuffer, matBuffer, normalGrassBuffer, grassBuffer, depthBuffer;
uniform vec2 pixelSize;

uniform mat4 invViewProjMatrix, invProjMatrix;

uniform mat4 lightViewProjNear, lightViewProjMid, lightViewProjFar;
uniform sampler2D depthBufferNear, depthBufferMid, depthBufferFar;
uniform vec2 shadowPixSize;
uniform int useShadow;
uniform vec2 levels;
uniform vec3 light;
uniform vec3 eyePos;
uniform float time;

in vec2 vTexcoord;

out vec4 FragColor;

#define GAP float(30.0)
#define INV2GAP float(0.01667)

/*
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
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float genPCF(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	int randround = 4;
	float shadowFactor = 1.0;
	float mag = 0.00143;
	float roundmag = 1.0 / float(randround);
	for (int i = 0; i < randround; i++) {
		int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
		float factor = texture2D(shadowMap, shadowCoord.xy + poissonDisk[index] * mag).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
		shadowFactor -= roundmag * (1.0 - factor);
	}
	return shadowFactor;
}

float genShadow(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	float shadowFactor = texture2D(shadowMap, shadowCoord.xy).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
	return shadowFactor;
}
//*/

///*
float genPCF(sampler2D shadowMap, vec3 shadowCoord, float bias, float pcount, float inv) {
	float shadowFactor = 0.0;
	for(float offx = -pcount; offx <= pcount; offx += 1.0) {
		for(float offy = -pcount; offy <= pcount; offy += 1.0) 
			shadowFactor += texture2D(shadowMap, shadowCoord.xy + vec2(offx, offy) * shadowPixSize).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
	}
	return shadowFactor * inv;
}


float genShadow(sampler2D shadowMap, vec3 shadowCoord, float bias) {
	return texture2D(shadowMap, shadowCoord.xy).r > (shadowCoord.z - bias) ? 1.0 : 0.0;
}
//*/

float genShadowFactor(vec4 worldPos, float depthView, float bias) {
	if(depthView <= levels.x - GAP) {
		vec4 near = lightViewProjNear * worldPos;
		vec3 lightPosition = near.xyz / near.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.00001;
		return genPCF(depthBufferNear, shadowCoord, bs, 2.0, 0.04);
	} else if(depthView > levels.x - GAP && depthView < levels.x + GAP) {
		vec4 near = lightViewProjNear * worldPos;
		vec3 lightPositionNear = near.xyz / near.w;
		vec3 shadowCoordNear = lightPositionNear * 0.5 + 0.5;
		float bsNear = bias * 0.00001;

		vec4 mid = lightViewProjMid * worldPos;
		vec3 lightPositionMid = mid.xyz / mid.w;
		vec3 shadowCoordMid = lightPositionMid * 0.5 + 0.5;
		float bsMid = bias * 0.00001;

		float factorNear = genPCF(depthBufferNear, shadowCoordNear, bsNear, 2.0, 0.04);
		float factorMid = genPCF(depthBufferMid, shadowCoordMid, bsMid, 1.0, 0.111);
		return mix(factorNear, factorMid, (depthView - (levels.x - GAP)) * INV2GAP);
	} else if(depthView <= levels.y) {
		vec4 mid = lightViewProjMid * worldPos;
		vec3 lightPosition = mid.xyz / mid.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.00001;
		return genPCF(depthBufferMid, shadowCoord, bs, 1.0, 0.111);
	} else {
		vec4 far = lightViewProjFar * worldPos;
		vec3 lightPosition = far.xyz / far.w;
		vec3 shadowCoord = lightPosition * 0.5 + 0.5;
		float bs = bias * 0.00001;
		return genShadow(depthBufferFar, shadowCoord, bs);
	}
	return 1.0;
}

float saturate(float value) {
	return clamp(value, 0.0, 1.0);
}

float BlendVal(float val, float val0, float val1, float res0, float res1) {
	if (val <= val0) return res0;
	if (val >= val1) return res1;
	return res0 + (val - val0) * (res1 - res0) / (val1 - val0);
}

vec3 Smudge(vec3 sceneTex, float grassFlag, float viewDist) {
	if(grassFlag < 0.5) 
		return sceneTex;
	else {
		float xx = vTexcoord.x;
		float yy = 1.0 - vTexcoord.y;

		float len = viewDist;
		float d = BlendVal(len, 0.0, 500.0, 100.0, 500.0);
		float dclose = BlendVal(len, 0.0, 20.0, 30.0, 1.0);
		d *= dclose;
		yy += dot(vec3(xx), vec3(1009.0, 1259.0, 2713.0));
		yy += time * 0.000004;
		yy += sceneTex.g * 0.04;
		
		float yoffset = 1.0 - fract(yy * d) / d * 1.5;
		vec2 uvoffset = vTexcoord - vec2(0.0, yoffset);
		vec4 grassColor = texture2D(texBuffer, uvoffset);

		float depthGrass = texture2D(depthBuffer, uvoffset).r;
		vec3 ndcGrass = vec3(uvoffset, depthGrass) * 2.0 - 1.0;
		vec4 viewGrass = invProjMatrix * vec4(ndcGrass, 1.0);
		viewGrass /= viewGrass.w;
		
		if(viewGrass.z > -viewDist)
			return sceneTex;
		else 
			return mix(sceneTex, grassColor.rgb, saturate(yoffset * d / 3.8));
	}
}

void main() {
	float depth = texture2D(depthBuffer, vTexcoord).r;
	vec3 ndcPos = vec3(vTexcoord, depth) * 2.0 - 1.0;
	vec4 tex = texture2D(texBuffer, vTexcoord);
	vec3 sceneColor = tex.rgb;

	if(ndcPos.z < 1.0) {
		vec4 worldPos = invViewProjMatrix * vec4(ndcPos, 1.0);
		worldPos /= worldPos.w;

		float depthView = distance(worldPos.xyz, eyePos);

		vec4 normalGrass = texture2D(normalGrassBuffer, vTexcoord);
		vec3 normal = normalGrass.xyz * 2.0 - vec3(1.0);
		vec3 material = texture2D(matBuffer, vTexcoord).rgb;

		float ndotl = dot(light, normal);
		float bias = tan(acos(abs(ndotl)));
		ndotl = max(ndotl, 0.0);

		float shadowFactor = (useShadow != 0) ? tex.a * genShadowFactor(worldPos, depthView, bias) : 1.0;

		float grassFlag = normalGrass.a;
		sceneColor = Smudge(sceneColor, grassFlag, depthView);
		sceneColor *= dot(material, vec3(1.0, shadowFactor * ndotl, 0.0));
	}

	FragColor = vec4(sceneColor, depth);
}
