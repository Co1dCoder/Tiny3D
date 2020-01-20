#include "shaderscontainer.h"
#include "../shader/textfile.h"
using namespace std;

#define SHADOW_TEX_FRAG "shader/shadow_tex.frag"
#define SHADOW_NONTEX_FRAG "shader/shadow_nontex.frag"
#define PHONG_VERT "shader/phong.vert"
#define PHONG_FRAG "shader/phong.frag"
#define INSTANCE_VERT "shader/instance.vert"
#define CULL_COMP "shader/cull.comp"
#define MULTI_COMP "shader/multiCull.comp"
#define FLUSH_COMP "shader/flush.comp"
#define BONE_VERT "shader/bone.vert"
#define BONE_FRAG "shader/bone.frag"
#define SKY_VERT "shader/sky.vert"
#define SKY_FRAG "shader/sky.frag"
#define POST_VERT "shader/post.vert"
#define DEFERRED_FRAG "shader/deferred.frag"
#define TERRAIN_VERT "shader/terrain.vert"
#define TERRAIN_FRAG "shader/terrain.frag"
#define BILLBOARD_VERT "shader/billboard.vert"
#define BILLBOARD_FRAG "shader/billboard.frag"
#define WATER_VERT "shader/water.vert"
#define WATER_FRAG "shader/water.frag"
#define AA_FRAG "shader/fxaa.frag"
#define BLUR_FRAG "shader/blur.frag"
#define MEAN_FRAG "shader/mean.frag"
#define GAUSS_FRAG "shader/gaussian.frag"
#define DOF_FRAG "shader/dof.frag"
#define DEBUG_VERT "shader/debug.vert"
#define DEBUG_FRAG "shader/debug.frag"
#define SCREEN_FRAG "shader/screen.frag"
#define SSR_FRAG "shader/ssr.frag"
#define COMBINE_FRAG "shader/combined.frag"
#define SSG_FRAG "shader/ssg.frag"
#define GRASS_LAYER_VERT "shader/grassLayer.vert"
#define GRASS_LAYER_FRAG "shader/grassLayer.frag"
#define GRASS_LAYER_GEOM "shader/grassLayer.geom"
#define GRASS_LAYER_TESC "shader/grassLayer.tesc"
#define GRASS_LAYER_TESE "shader/grassLayer.tese"
#define GRASS_COMP "shader/grass.comp"
#define GRASS_VERT "shader/grass.vert"
#define UTIL_GLSL "shader/util.glsl" 
#define TRIANGLE_GLSL "shader/triangle.glsl"

string LoadExShader(char* name) {
	char* fileStr = textFileRead(name);
	string shaderStr = string(fileStr);
	free(fileStr);
	return shaderStr;
}

void SetupShaders(ShaderManager* shaders) {
	string shaderUtil = LoadExShader(UTIL_GLSL);

	Shader* phong = shaders->addShader("phong", PHONG_VERT, PHONG_FRAG);
	phong->attachEx(shaderUtil);
	shaders->addShaderBindTex(phong);

	Shader* phongIns = shaders->addShader("phong_ins", INSTANCE_VERT, PHONG_FRAG);
	phongIns->attachEx(shaderUtil);
	shaders->addShaderBindTex(phongIns);

	Shader* bone = shaders->addShader("bone", BONE_VERT, BONE_FRAG);
	bone->attachEx(shaderUtil);
	shaders->addShaderBindTex(bone);

	Shader* billboard = shaders->addShader("billboard", BILLBOARD_VERT, BILLBOARD_FRAG);
	billboard->attachEx(shaderUtil);
	shaders->addShaderBindTex(billboard);

	Shader* terrain = shaders->addShader("terrain", TERRAIN_VERT, TERRAIN_FRAG);
	terrain->attachEx(shaderUtil);
	shaders->addShaderBindTex(terrain);

	Shader* grassLayer = shaders->addShader("grassLayer", GRASS_LAYER_VERT, GRASS_LAYER_FRAG, GRASS_LAYER_TESC, GRASS_LAYER_TESE, GRASS_LAYER_GEOM);
	grassLayer->attachEx(shaderUtil);
	shaders->addShaderBindTex(grassLayer);

	string shaderTriangle = LoadExShader(TRIANGLE_GLSL);
	Shader* grassComp = shaders->addShader("grassComp", GRASS_COMP);
	grassComp->attachDef("WORKGROUP_SIZE", to_string(COMP_GROUPE_SIZE).data());
	grassComp->attachEx(shaderUtil);
	grassComp->attachEx(shaderTriangle);

	Shader* grass = shaders->addShader("grass", GRASS_VERT, GRASS_LAYER_FRAG);
	grass->attachEx(shaderUtil);

	Shader* sky = shaders->addShader("sky", SKY_VERT, SKY_FRAG);
	sky->attachEx(shaderUtil);

	Shader* water = shaders->addShader("water", WATER_VERT, WATER_FRAG);
	water->attachEx(shaderUtil);

	Shader* phongShadow = shaders->addShader("phong_s", PHONG_VERT, SHADOW_TEX_FRAG);
	phongShadow->attachEx(shaderUtil);
	phongShadow->attachDef("ShadowPass", "1.0");
	shaders->addShaderBindTex(phongShadow);

	Shader* phongShadowIns = shaders->addShader("phong_s_ins", INSTANCE_VERT, SHADOW_TEX_FRAG);
	phongShadowIns->attachEx(shaderUtil);
	phongShadowIns->attachDef("ShadowPass", "1.0");
	shaders->addShaderBindTex(phongShadowIns);

	Shader* phongShadowLow = shaders->addShader("phong_sl", PHONG_VERT, SHADOW_NONTEX_FRAG);
	phongShadowLow->attachEx(shaderUtil);
	phongShadowLow->attachDef("ShadowPass", "1.0");
	phongShadowLow->attachDef("LowPass", "1.0");

	Shader* phongSimpShadowLow = shaders->addShader("phong_sl_ins", INSTANCE_VERT, SHADOW_NONTEX_FRAG);
	phongSimpShadowLow->attachEx(shaderUtil);
	phongSimpShadowLow->attachDef("ShadowPass", "1.0");
	phongSimpShadowLow->attachDef("LowPass", "1.0");

	Shader* boneShadow = shaders->addShader("bone_s", BONE_VERT, SHADOW_NONTEX_FRAG);
	boneShadow->attachEx(shaderUtil);
	boneShadow->attachDef("ShadowPass", "1.0");

	Shader* billboardShadow = shaders->addShader("billboard_s", BILLBOARD_VERT, SHADOW_TEX_FRAG);
	billboardShadow->attachEx(shaderUtil);
	billboardShadow->attachDef("ShadowPass", "1.0");
	shaders->addShaderBindTex(billboardShadow);

	Shader* deferred = shaders->addShader("deferred", POST_VERT, DEFERRED_FRAG);
	deferred->attachEx(shaderUtil);
	deferred->setSlot("texBuffer", 0);
	deferred->setSlot("matBuffer", 1);
	deferred->setSlot("normalGrassBuffer", 2);
	deferred->setSlot("roughMetalBuffer", 3);
	deferred->setSlot("depthBuffer", 4);

	Shader* fxaa = shaders->addShader("fxaa", POST_VERT, AA_FRAG);
	fxaa->attachEx(shaderUtil);
	fxaa->setSlot("colorBuffer", 0);
	fxaa->setSlot("normalBuffer", 1);
	fxaa->setSlot("depthBuffer", 2);

	Shader* blur = shaders->addShader("blur", POST_VERT, BLUR_FRAG);
	blur->attachEx(shaderUtil);
	blur->setSlot("colorBuffer", 0);

	Shader* mean = shaders->addShader("mean", POST_VERT, MEAN_FRAG);
	mean->attachEx(shaderUtil);
	mean->setSlot("colorBuffer", 0);

	Shader* gauss = shaders->addShader("gauss", POST_VERT, GAUSS_FRAG);
	gauss->attachEx(shaderUtil);
	gauss->setSlot("colorBuffer", 0);

	Shader* dof = shaders->addShader("dof", POST_VERT, DOF_FRAG);
	dof->attachEx(shaderUtil);
	dof->setSlot("colorBufferLow", 0);
	dof->setSlot("colorBufferHigh", 1);

	Shader* debug = shaders->addShader("debug", DEBUG_VERT, DEBUG_FRAG);
	debug->attachEx(shaderUtil);
	
	Shader* screen = shaders->addShader("screen", POST_VERT, SCREEN_FRAG);
	screen->attachEx(shaderUtil);
	screen->setSlot("tex", 0);

	Shader* ssr = shaders->addShader("ssr", POST_VERT, SSR_FRAG);
	ssr->attachEx(shaderUtil);
	ssr->setSlot("lightBuffer", 0);
	ssr->setSlot("matBuffer", 1);
	ssr->setSlot("normalBuffer", 2);
	ssr->setSlot("depthBuffer", 3);

	Shader* combined = shaders->addShader("combined", POST_VERT, COMBINE_FRAG);
	combined->attachEx(shaderUtil);
	combined->setSlot("sceneBuffer", 0);
	combined->setSlot("sceneDepthBuffer", 1);
	combined->setSlot("waterBuffer", 2);
	combined->setSlot("waterDepthBuffer", 3);
	combined->setSlot("matBuffer", 4);
	combined->setSlot("waterNormalBuffer", 5);
	combined->setSlot("bloomBuffer", 6);

	Shader* ssg = shaders->addShader("ssg", POST_VERT, SSG_FRAG);
	ssg->attachEx(shaderUtil);
	ssg->setSlot("colorBuffer", 0);
	ssg->setSlot("normalGrassBuffer", 1);
	ssg->setSlot("depthBuffer", 2);

	Shader* cull = shaders->addShader("cull", CULL_COMP);
	cull->attachDef("WORKGROUP_SIZE", to_string(WORKGROUPE_SIZE).data());
	cull->attachEx(shaderUtil);

	Shader* multi = shaders->addShader("multi", MULTI_COMP);
	multi->attachDef("WORKGROUP_SIZE", to_string(WORKGROUPE_SIZE).data());
	multi->attachDef("MAX_DISPATCH", to_string(MAX_DISPATCH).data());
	multi->attachEx(shaderUtil);

	Shader* flush = shaders->addShader("flush", FLUSH_COMP);
	flush->attachEx(shaderUtil);

	shaders->compile();
}

