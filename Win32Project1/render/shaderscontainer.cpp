#include "shaderscontainer.h"

#define PHONG_VERT "shader/phong.vert"
#define PHONG_FRAG "shader/phong.frag"
#define PHONG_INS_VERT "shader/phong_ins.vert"
#define PHONG_INS_FRAG "shader/phong_ins.frag"
#define BONE_VERT "shader/bone.vert"
#define BONE_FRAG "shader/bone.frag"
#define PHONG_SHADOW_VERT "shader/phong_shadow.vert"
#define PHONG_SHADOW_INS_VERT "shader/phong_shadow_ins.vert"
#define PHONG_SHADOW_FRAG "shader/phong_shadow.frag"
#define PHONG_SHADOW_LOW_VERT "shader/phong_shadow_low.vert"
#define PHONG_SHADOW_LOW_INS_VERT "shader/phong_shadow_low_ins.vert"
#define PHONG_SHADOW_LOW_FRAG "shader/phong_shadow_low.frag"
#define BONE_SHADOW_VERT "shader/bone_shadow.vert"
#define BONE_SHADOW_FRAG "shader/bone_shadow.frag"
#define SKY_VERT "shader/sky.vert"
#define SKY_FRAG "shader/sky.frag"
#define DEFERRED_VERT "shader/deferred.vert"
#define DEFERRED_FRAG "shader/deferred.frag"
#define TERRAIN_VERT "shader/terrain.vert"
#define TERRAIN_FRAG "shader/terrain.frag"
#define BILLBOARD_VERT "shader/billboard.vert"
#define BILLBOARD_FRAG "shader/billboard.frag"
#define BILLBOARD_SHADOW_VERT "shader/billboard_shadow.vert"
#define BILLBOARD_SHADOW_FRAG "shader/billboard_shadow.frag"
#define WATER_VERT "shader/water.vert"
#define WATER_FRAG "shader/water.frag"
#define AA_VERT "shader/fxaa.vert"
#define AA_FRAG "shader/fxaa.frag"
#define BLUR_VERT "shader/blur.vert"
#define BLUR_FRAG "shader/blur.frag"
#define DOF_VERT "shader/dof.vert"
#define DOF_FRAG "shader/dof.frag"
#define DEBUG_VERT "shader/debug.vert"
#define DEBUG_FRAG "shader/debug.frag"

void SetupShaders(Render* render) {
	Shader* phong=render->shaders->addShader("phong",PHONG_VERT,PHONG_FRAG);
	render->useShader(phong);
	phong->setSampler("texture",0);

	Shader* phongIns = render->shaders->addShader("phong_ins", PHONG_INS_VERT, PHONG_INS_FRAG);
	render->useShader(phongIns);
	phongIns->setSampler("texture", 0);

	Shader* terrain = render->shaders->addShader("terrain", TERRAIN_VERT, TERRAIN_FRAG);
	render->useShader(terrain);
	terrain->setSampler("texture", 0);

	Shader* bone=render->shaders->addShader("bone",BONE_VERT,BONE_FRAG);
	render->useShader(bone);
	bone->setSampler("texture",0);

	Shader* phongShadow = render->shaders->addShader("phong_s", PHONG_SHADOW_VERT, PHONG_SHADOW_FRAG);
	render->useShader(phongShadow);
	phongShadow->setSampler("texture", 0);

	Shader* phongShadowIns = render->shaders->addShader("phong_s_ins", PHONG_SHADOW_INS_VERT, PHONG_SHADOW_FRAG);
	render->useShader(phongShadowIns);
	phongShadowIns->setSampler("texture", 0);

	render->shaders->addShader("phong_sl", PHONG_SHADOW_LOW_VERT, PHONG_SHADOW_LOW_FRAG);
	render->shaders->addShader("phong_sl_ins", PHONG_SHADOW_LOW_INS_VERT, PHONG_SHADOW_LOW_FRAG);
	render->shaders->addShader("bone_s", BONE_SHADOW_VERT, BONE_SHADOW_FRAG);

	Shader* sky=render->shaders->addShader("sky",SKY_VERT,SKY_FRAG);
	render->useShader(sky);
	sky->setSampler("textureSky",0);

	Shader* billboard = render->shaders->addShader("billboard", BILLBOARD_VERT, BILLBOARD_FRAG);
	render->useShader(billboard);
	billboard->setSampler("texture", 0);

	Shader* billboardShadow = render->shaders->addShader("billboard_s", BILLBOARD_SHADOW_VERT, BILLBOARD_SHADOW_FRAG);
	render->useShader(billboardShadow);
	billboardShadow->setSampler("texture", 0);

	Shader* water = render->shaders->addShader("water", WATER_VERT, WATER_FRAG);
	render->useShader(water);
	sky->setSampler("textureEnv", 0);

	Shader* deferred = render->shaders->addShader("deferred", DEFERRED_VERT, DEFERRED_FRAG);
	render->useShader(deferred);
	deferred->setSampler("texBuffer", 0);
	deferred->setSampler("colorBuffer", 1);
	deferred->setSampler("normalBuffer", 2);
	deferred->setSampler("depthBuffer", 3);
	deferred->setSampler("depthBufferNear", 4);
	deferred->setSampler("depthBufferMid", 5);
	deferred->setSampler("depthBufferFar", 6);

	Shader* fxaa = render->shaders->addShader("fxaa", AA_VERT, AA_FRAG);
	render->useShader(fxaa);
	fxaa->setSampler("colorBuffer", 0);

	Shader* blur = render->shaders->addShader("blur", BLUR_VERT, BLUR_FRAG);
	render->useShader(blur);
	blur->setSampler("colorBuffer", 0);

	Shader* dof = render->shaders->addShader("dof", DOF_VERT, DOF_FRAG);
	render->useShader(dof);
	dof->setSampler("colorBufferLow", 0);
	dof->setSampler("colorBufferHigh", 1);

	Shader* debug = render->shaders->addShader("debug", DEBUG_VERT, DEBUG_FRAG);
}

