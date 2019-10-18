uniform mat4 viewMatrix;
uniform mat4 projectMatrix, invProjMatrix;
layout(bindless_sampler) uniform sampler2D lightBuffer, matBuffer, normalBuffer, depthBuffer;
uniform vec2 screenSize, pixelSize;

in vec2 vTexcoord;

out vec4 ReflectColor;

vec2 BinarySearch(float start, float end, vec2 projUV, vec3 refDir, vec3 refPos, vec4 projRef) {
	float startLen = start, endLen = end;
	float curLen = start + (end - start) * 0.5;
	vec2 uvCoord = projUV;

	for(int i = 0; i < 3; i++) {
		vec3 curPos = refPos + refDir * curLen;

		vec4 projPos = projectMatrix * vec4(curPos, 1.0);
		projPos /= projPos.w;
		vec3 projCoord = projPos.xyz * 0.5 + 0.5;

		float storedDepth = texture2D(depthBuffer, projCoord.xy).r;

		if(storedDepth <= projCoord.z) {
			endLen = curLen;
			uvCoord = projCoord.xy;
		} else 
			startLen = curLen;
		curLen = startLen + (endLen - startLen) * 0.5;
	}

	return uvCoord;
}

vec4 RayCast(vec3 refDir, vec3 refPos) {
	float stepLen = 200.0;
	float lenStart = 0.0;
	float lenBefore = lenStart;
	float lenCurrent = lenStart;

	float border = 0.1;
	float oneMinBorder = 0.9;
	float invBorder = 10.0;
	
	vec4 projRef = projectMatrix * vec4(refPos, 1.0);
	projRef /= projRef.w;
	
	for(float i = 0.0; i < 10.0; i += 1.0) {
		vec3 curPos = refPos + refDir * lenCurrent;

		vec4 projPos = projectMatrix * vec4(curPos, 1.0);
		projPos /= projPos.w;
		vec3 projCoord = projPos.xyz * 0.5 + 0.5;

		float storedDepth = texture2D(depthBuffer, projCoord.xy).r;
		vec4 storedView = invProjMatrix * vec4(vec3(projCoord.xy, storedDepth) * 2.0 - 1.0, 1.0);
		storedView /= storedView.w;

		float refFlag = texture2D(matBuffer, projCoord.xy).a;

		if(storedDepth >= 1.0)
			return FAIL_COLOR;
		else if(curPos.z <= storedView.z + 0.01 && refFlag > 0.9 && refPos.z >= storedView.z - 20.0) {
			vec2 searchData = BinarySearch(lenBefore, lenCurrent, projCoord.xy, refDir, refPos, projRef);
			vec4 storedData = texture2D(lightBuffer, searchData);

			if(searchData.y >= oneMinBorder)
				storedData = mix(storedData, FAIL_COLOR, (searchData.y - oneMinBorder) * invBorder);

			if(searchData.x >= oneMinBorder)
				storedData = mix(storedData, FAIL_COLOR, (searchData.x - oneMinBorder) * invBorder);
			else if(searchData.x <= border)
				storedData = mix(storedData, FAIL_COLOR, (border - searchData.x) * invBorder);

			return storedData; 
		} else {
			lenBefore = lenCurrent;			
			lenCurrent = lenStart + stepLen * (i + random(gl_FragCoord.xyz, i));
		}
	}

	return FAIL_COLOR;
}

void main() {
	float refFlag = texture2D(matBuffer, vTexcoord).a;

	if(refFlag > 0.9)
		ReflectColor = texture2D(lightBuffer, vTexcoord);
	else {
		float depth = texture2D(depthBuffer, vTexcoord).r;
		vec3 ndcPos = vec3(vTexcoord, depth) * 2.0 - 1.0;

		vec4 viewPos = invProjMatrix * vec4(ndcPos, 1.0);
		viewPos /= viewPos.w;
		
		vec3 normal = texture2D(normalBuffer, vTexcoord).xyz * 2.0 - 1.0;
		vec3 viewNormal = mat3(viewMatrix) * normal;
		
		vec3 reflectDir = normalize(reflect(viewPos.xyz, viewNormal));
		ReflectColor = RayCast(reflectDir, viewPos.xyz);
	}
	ReflectColor.rgb = pow(ReflectColor.rgb, GAMMA);
}