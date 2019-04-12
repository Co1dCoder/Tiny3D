#include "render.h"
#include "../constants/constants.h"
#include "../assets/assetManager.h"
#include "instanceDrawcall.h"
#include <stdio.h>

Render::Render() {
	shaders=new ShaderManager();
	initEnvironment();
	tmpState = new RenderState();
	needRevertState = false;
}

Render::~Render() {
	delete tmpState;
	delete shaders;
	shaders=NULL;
	clearTextureSlots();
}

float Render::MaxAniso = 0.0;

void Render::initEnvironment() {
	GLenum err=glewInit();
	if(GLEW_OK!=err)
		printf("Error: %s\n",glewGetErrorString(err));
	// Force to refresh states
	enableDepthTest=false;
	depthTestMode=GREATER;
	enableAlphaTest = true;
	alphaTestMode = LESS;
	alphaThreshold = 1.0;
	enableCull=false;
	cullMode=CULL_NONE;
	drawLine=true;
	enableBlend = true;
	setDepthTest(true,LEQUAL);
	setAlphaTest(false, GREATER, 0);
	setCullState(true);
	setCullMode(CULL_BACK);
	setDrawLine(false);
	setBlend(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setClearColor(1,1,1,1);
	currentShader=NULL;
	clearTextureSlots();

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAniso);
}

void Render::clearFrame(bool clearColor,bool clearDepth,bool clearStencil) {
	GLbitfield clearMask=0;
	if(clearColor) clearMask|=GL_COLOR_BUFFER_BIT;
	if(clearDepth) clearMask|=GL_DEPTH_BUFFER_BIT;
	if(clearStencil) clearMask|=GL_STENCIL_BUFFER_BIT;
	glClear(clearMask);
}

void Render::setState(const RenderState* state) {
	setDepthTest(state->enableDepthTest,state->depthTestMode);
	setAlphaTest(state->enableAlphaTest, state->alphaTestMode, state->alphaThreshold);
	setCullState(state->enableCull);
	setCullMode(state->cullMode);
	setDrawLine(state->drawLine);
	setBlend(state->blend);
}

void Render::setDepthTest(bool enable,int testMode) {
	if(enable!=enableDepthTest) {
		enableDepthTest=enable;
		if(enable) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	}
	if(testMode!=depthTestMode) {
		depthTestMode=testMode;
		switch(testMode) {
			case LESS:
				glDepthFunc(GL_LESS);
				break;
			case LEQUAL:
				glDepthFunc(GL_LEQUAL);
				break;
			case GREATER:
				glDepthFunc(GL_GREATER);
				break;
			case GEQUAL:
				glDepthFunc(GL_GEQUAL);
				break;
		}
	}
}

void Render::setAlphaTest(bool enable, int testMode, float threshold) {
	if (enable != enableAlphaTest) {
		enableAlphaTest = enable;
		if (enable) glEnable(GL_ALPHA_TEST);
		else glDisable(GL_ALPHA_TEST);
	}
	if (testMode != alphaTestMode || threshold != alphaThreshold) {
		alphaTestMode = testMode;
		alphaThreshold = threshold;
		switch (testMode) {
			case LESS:
				glAlphaFunc(GL_LESS, alphaThreshold);
				break;
			case LEQUAL:
				glAlphaFunc(GL_LEQUAL, alphaThreshold);
				break;
			case GREATER:
				glAlphaFunc(GL_GREATER, alphaThreshold);
				break;
			case GEQUAL:
				glAlphaFunc(GL_GEQUAL, alphaThreshold);
				break;
		}
	}
}

void Render::setCullState(bool enable) {
	if(enable==enableCull) return;
	enableCull=enable;
	if(enable) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
}

void Render::setCullMode(int mode) {
	if(mode==cullMode) return;
	cullMode=mode;
	switch(mode) {
		case CULL_BACK:
			glCullFace(GL_BACK);
			break;
		case CULL_FRONT:
			glCullFace(GL_FRONT);
			break;
		case CULL_NONE:
			glCullFace(GL_NONE);
			break;
	}
}

void Render::setDrawLine(bool line) {
	if(line==drawLine) return;
	drawLine=line;
	if(!line) glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	else glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
}

void Render::setBlend(bool enable) {
	if (enable == enableBlend) return;
	enableBlend = enable;
	if (enable) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
}

void Render::setClearColor(float r,float g,float b,float a) {
	clearColor.r=r; clearColor.g=g;
	clearColor.b=b; clearColor.a=a;
	glClearColor(r,g,b,a);
}

void Render::setViewPort(int width,int height) {
	viewWidth=width; viewHeight=height;
	glViewport(0,0,width,height);
}

void Render::resize(int width, int height, Camera* mainCamera, Camera* reflectCamera) {
	if (height == 0) height = 1;
	setViewPort(width, height);
	float fAspect = (float)width / height;
	mainCamera->initPerspectCamera(60.0, fAspect, 1.0, 2000.0);
	mainCamera->initPerspectSub(1000.0);
	mainCamera->initPerspectNear(300.0);
	if (reflectCamera) 
		reflectCamera->initPerspectCamera(60.0, fAspect, 0.1, 2000.0);
}

Shader* Render::findShader(const char* shader) {
	return shaders->findShader(shader);
}

void Render::useShader(Shader* shader) {
	if (shader == currentShader) return;
	currentShader = shader;
	shader->use();
}

// Pass 1 draw near shadow
// Pass 2 draw mid shadow
// Pass 3 draw far shadow (use simple buffer)
// Pass 4 draw scene with color
// Pass 5 deferred process
// Pass 6 fxaa dof ssr
void Render::draw(Camera* camera,Drawcall* drawcall,RenderState* state) {
	needRevertState = false;
	if (drawcall->isSingleSide()) { // Special case
		tmpState->copy(state);
		needRevertState = true;
		if (state->pass == COLOR_PASS)
			state->enableCull = false;
		else
			state->cullMode = CULL_BACK;
	}
	if (drawcall->isBillboard()) {
		if (!needRevertState) tmpState->copy(state);
		needRevertState = true;
		state->enableCull = false;
	}

	setState(state);

	Shader* shader = NULL;
	if (drawcall->isBillboard() && state->shaderBillboard) shader = state->shaderBillboard; 
	else {
		shader = drawcall->getType() == INSTANCE_DC ? state->shaderIns : state->shader;
		if (drawcall->getType() == INSTANCE_DC) {
			if (state->grass)
				shader = state->shaderGrass;
			else if (state->simpleIns)
				shader = state->shaderSimple;		
		}
	}
	useShader(shader);
	if (camera) {
		if (state->pass < DEFERRED_PASS) {
			if (!state->skyPass) {
				shader->setMatrix4("viewProjectMatrix", camera->viewProjectMatrix);
				if (state->grass) {
					shader->setFloat("time", state->time * 0.05);
					shader->setVector3v("eyePos", *(state->eyePos));
					shader->setMatrix4("viewMatrix", camera->viewMatrix);
				}

				if (drawcall->isBillboard()) {
					if (state->pass != COLOR_PASS)
						shader->setVector3("viewRight", state->light.z, 0.0, -state->light.x);
					else if (state->pass == COLOR_PASS) {
						static VECTOR3D upVec(0.0, 1.0, 0.0);
						VECTOR3D viewRight(camera->viewMatrix.entries[0], camera->viewMatrix.entries[4], camera->viewMatrix.entries[8]);
						VECTOR3D normal = (viewRight.CrossProduct(upVec)).GetNormalized();
						normal.x = normal.x * 0.5 + 0.5;
						normal.y = normal.y * 0.5 + 0.5;
						normal.z = normal.z * 0.5 + 0.5;
						shader->setVector3v("uNormal", normal);
						shader->setVector3v("viewRight", viewRight);
					} 
				}

				if (state->pass != FAR_SHADOW_PASS || drawcall->isBillboard())  // use texture atlas
					shader->setVector4v("texPixel", AssetManager::assetManager->texAlt->atlasInfo);
				if (state->waterPass) {
					shader->setFloat("time", state->time);
					shader->setVector3v("eyePos", *(state->eyePos));
					shader->setVector3("light", -state->light.x, -state->light.y, -state->light.z);
					shader->setMatrix4("viewMatrix", camera->viewMatrix);
					if (state->enableSsr)
						shader->setVector2("waterBias", 0.0, 0.0);
					else
						shader->setVector2("waterBias", 0.05, -0.05);
					if (AssetManager::assetManager->getSkyTexture())
						useTexture(TEXTURE_CUBE, 1, AssetManager::assetManager->getSkyTexture()->id);
					if (AssetManager::assetManager->getReflectTexture())
						useTexture(TEXTURE_2D, 2, AssetManager::assetManager->getReflectTexture()->id);
				}
			} else {
				shader->setMatrix4("viewMatrix", camera->viewMatrix);
				shader->setMatrix4("projectMatrix", camera->projectMatrix);
			}
		} else if (state->pass == DEFERRED_PASS) {
			shader->setMatrix4("invViewProjMatrix", camera->invViewProjectMatrix);
			shader->setMatrix4("invProjMatrix", camera->invProjMatrix);
			shader->setMatrix4("viewMatrix", camera->viewMatrix);
			shader->setVector3v("eyePos", *(state->eyePos));
			shader->setFloat("time", state->time);

			if (state->shadow) {
				shader->setMatrix4("lightViewProjNear", state->shadow->lightNearMat);
				shader->setMatrix4("lightViewProjMid", state->shadow->lightMidMat);
				shader->setMatrix4("lightViewProjFar", state->shadow->lightFarMat);
				shader->setVector2("levels", state->shadow->level1, state->shadow->level2);
			}
			if (state->lightEffect) 
				shader->setVector3("light", -state->light.x, -state->light.y, -state->light.z);
		} 
		if (state->ssrPass) {
			shader->setMatrix4("viewMatrix", camera->viewMatrix);
			shader->setMatrix4("projectMatrix", camera->projectMatrix);
			shader->setMatrix4("invProjMatrix", camera->invProjMatrix);
			shader->setVector2("screenSize", viewWidth, viewHeight);
		}
	}

	drawcall->draw(shader, state->pass);

	if (needRevertState) state->copy(tmpState); // Reset state
}

void Render::finishDraw() {
	//glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Render::setFrameBuffer(FrameBuffer* framebuffer) {
	if(framebuffer) framebuffer->use();
	else {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
		clearFrame(true,true,false);
		glViewport(0,0,viewWidth,viewHeight);
	}
}

void Render::setColorMask(bool r, bool g, bool b, bool a) {
	glColorMask(r, g, b, a);
}

void Render::useTexture(uint type, uint slot, uint texid) {
	std::map<uint, std::map<uint, uint>*>::iterator typeItor = textureTypeSlots.find(type);
	std::map<uint, uint>* textureInUse = NULL;
	if (typeItor != textureTypeSlots.end())
		textureInUse = typeItor->second;
	else {
		std::map<uint, uint>* slotMap = new std::map<uint, uint>();
		textureTypeSlots[type] = slotMap;
		slotMap->clear();
		textureInUse = slotMap;
	}

	std::map<uint, uint>::iterator texItor = textureInUse->find(slot);
	if (texItor != textureInUse->end() && texItor->second == texid) 
		return;

	GLenum textureType = GL_TEXTURE_2D;
	switch (type) {
		case TEXTURE_2D:
			textureType = GL_TEXTURE_2D;
			break;
		case TEXTURE_2D_ARRAY:
			textureType = GL_TEXTURE_2D_ARRAY;
			break;
		case TEXTURE_CUBE:
			textureType = GL_TEXTURE_CUBE_MAP;
			break;
	}
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(textureType, texid);
	(*textureInUse)[slot] = texid;
}


void Render::clearTextureSlots() {
	std::map<uint, std::map<uint, uint>*>::iterator itor = textureTypeSlots.begin();
	while (itor != textureTypeSlots.end()) {
		delete itor->second;
		++itor;
	}
	textureTypeSlots.clear();
}

