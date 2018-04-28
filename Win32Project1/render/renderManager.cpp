#include "renderManager.h"
#include "../assets/assetManager.h"

RenderManager::RenderManager(Camera* view, float distance1, float distance2, const VECTOR3D& light) {
	shadow=new Shadow(view,distance1,distance2);
	float nearSize=1024;
	float midSize=2048;
	float farSize=2048;
	nearBuffer = new FrameBuffer(nearSize, nearSize, HIGH_PRE, 4);
	midBuffer = new FrameBuffer(midSize, midSize, HIGH_PRE, 4);
	farBuffer = new FrameBuffer(farSize, farSize, HIGH_PRE, 4);
	lightDir = light; lightDir.Normalize();

	renderData = new Renderable(distance1, distance2); renderData->copyCamera(view);
	queue1 = new Renderable(distance1, distance2); queue1->copyCamera(view);
	queue2 = new Renderable(distance1, distance2); queue2->copyCamera(view);
	currentQueue = queue1;
	nextQueue = queue2;

	useShadow = false;
	drawBounding = true;
	state = new RenderState();

	phongShadow = NULL;
	phongShadowLow = NULL;
	phong = NULL;
	phongShadowIns = NULL;
	phongShadowLowIns = NULL;
	phongIns = NULL;
	boneShadow = NULL;
	bone = NULL;
	mix = NULL;
	skyCube = NULL;
	deferred = NULL;
}

RenderManager::~RenderManager() {
	delete shadow; shadow=NULL;
	delete nearBuffer; nearBuffer=NULL;
	delete midBuffer; midBuffer=NULL;
	delete farBuffer; farBuffer=NULL;

	delete renderData; renderData = NULL;
	delete queue1; queue1 = NULL;
	delete queue2; queue2 = NULL;

	delete state; state = NULL;
}

void RenderManager::updateShadowCamera() {
	shadow->prepareViewCamera();
}

void RenderManager::updateMainLight() {
	lightDir.Normalize();
	shadow->update(lightDir);
}

void RenderManager::flushRenderQueues() {
	renderData->flush();
}

void RenderManager::updateRenderQueues(Scene* scene) {
	Camera* cameraNear = shadow->lightCameraNear;
	Camera* cameraMid = shadow->lightCameraMid;
	Camera* cameraFar = shadow->lightCameraFar;
	Camera* cameraMain = scene->mainCamera;

	pushNodeToQueue(renderData->shadowNearStaticQueue, scene->staticRoot, cameraNear);
	pushNodeToQueue(renderData->shadowMidStaticQueue, scene->staticRoot, cameraMid);
	pushNodeToQueue(renderData->shadowFarStaticQueue, scene->staticRoot, cameraFar);
	pushNodeToQueue(renderData->staticQueue, scene->staticRoot, cameraMain);
	pushNodeToQueue(renderData->shadowNearAnimateQueue, scene->animationRoot, cameraNear);
	pushNodeToQueue(renderData->shadowMidAnimateQueue, scene->animationRoot, cameraMid);
	pushNodeToQueue(renderData->shadowFarAnimateQueue, scene->animationRoot, cameraFar);
	pushNodeToQueue(renderData->animateQueue, scene->animationRoot, cameraMain);
}

void RenderManager::animateQueues(long startTime, long currentTime) {
	currentQueue->shadowNearAnimateQueue->animate(startTime, currentTime);
	currentQueue->shadowMidAnimateQueue->animate(startTime, currentTime);
	currentQueue->shadowFarAnimateQueue->animate(startTime, currentTime);
	currentQueue->animateQueue->animate(startTime, currentTime);
}

void RenderManager::swapRenderQueues(Scene* scene) {
	flushRenderQueues();
	updateRenderQueues(scene);
	nextQueue->flush();
	nextQueue->copyData(renderData);

	currentQueue = (currentQueue == queue1) ? queue2 : queue1;
	nextQueue = (nextQueue == queue1) ? queue2 : queue1;
}

void RenderManager::renderShadow(Render* render, Scene* scene) {
	if (!useShadow) return;

	state->reset();
	state->cullMode = CULL_FRONT;
	state->enableAlphaTest = true;
	state->alphaThreshold = 0.0;
	state->alphaTestMode = GREATER;

	if (!phongShadow) phongShadow = render->findShader("phong_s");
	if (!phongShadowLow) phongShadowLow = render->findShader("phong_sl");
	if (!phongShadowIns) phongShadowIns = render->findShader("phong_s_ins");
	if (!phongShadowLowIns) phongShadowLowIns = render->findShader("phong_sl_ins");
	if (!boneShadow) boneShadow = render->findShader("bone_s");

	Camera* mainCamera = scene->mainCamera;

	render->useTexture(TEXTURE_2D_ARRAY, 0, AssetManager::assetManager->textures->setId);

	render->setFrameBuffer(nearBuffer);
	Camera* cameraNear=shadow->lightCameraNear;
	state->pass = 1;
	state->shader = phongShadow;
	state->shaderIns = phongShadowIns;
	currentQueue->shadowNearStaticQueue->draw(cameraNear, mainCamera->position, render, state);
	state->shader = boneShadow;
	currentQueue->shadowNearAnimateQueue->draw(cameraNear, mainCamera->position, render, state);

	render->setFrameBuffer(midBuffer);
	Camera* cameraMid=shadow->lightCameraMid;
	state->pass = 2;
	state->shader = phongShadow;
	currentQueue->shadowMidStaticQueue->draw(cameraMid, mainCamera->position, render, state);
	state->shader = boneShadow;
	currentQueue->shadowMidAnimateQueue->draw(cameraMid, mainCamera->position, render, state);

	static ushort flushCount = 1;
	if (flushCount % 2 == 0) 
		flushCount = 1;
	else {
		render->setFrameBuffer(farBuffer);
		Camera* cameraFar = shadow->lightCameraFar;
		state->pass = 3;
		state->shader = phongShadowLow;
		state->shaderIns = phongShadowLowIns;
		currentQueue->shadowFarStaticQueue->draw(cameraFar, mainCamera->position, render, state);
		state->shader = boneShadow;
		currentQueue->shadowFarAnimateQueue->draw(cameraFar, mainCamera->position, render, state);
		
		flushCount++;
	}
}

void RenderManager::renderScene(Render* render, Scene* scene) {
	state->reset();
	state->enableAlphaTest = true;
	state->alphaThreshold = 0.0;
	state->alphaTestMode = GREATER;

	if (!phong) phong = render->findShader("phong");
	if (!phongIns) phongIns = render->findShader("phong_ins");
	if (!bone) bone = render->findShader("bone");
	if (!mix) mix = render->findShader("terrain");
	if (!skyCube) skyCube = render->findShader("sky");

	//Camera* camera=currentQueue->mainCamera; // Exchange camera will lead delay
	Camera* camera = scene->mainCamera;
	render->useTexture(TEXTURE_2D_ARRAY, 0, AssetManager::assetManager->textures->setId);
	state->shader = phong;
	state->shaderIns = phongIns;
	currentQueue->staticQueue->draw(camera, camera->position, render, state);

	// Draw terrain
	if (scene->terrainNode) {
		state->shader = mix;
		if (!scene->terrainNode->needUpdateNode) {
			if (!scene->terrainNode->needCreateDrawcall) 
				render->draw(camera, scene->terrainNode->drawcall, state);
			else
				scene->terrainNode->prepareDrawcall();
		}
	}

	state->shader = bone;
	currentQueue->animateQueue->draw(camera, camera->position, render, state);

	// Debug mode
	if (drawBounding) {
		scene->clearAllAABB();
		scene->createNodeAABB(scene->staticRoot);
		scene->createNodeAABB(scene->animationRoot);

		state->enableCull = false;
		state->drawLine = true;
		state->shader = phong;
		drawBoundings(render, state, scene, camera);
	}

	// Draw sky
	if (scene->skyBox) 
		scene->skyBox->draw(render, skyCube, camera);

	scene->flushNodes();
}

void RenderManager::drawDeferred(Render* render, Scene* scene, FrameBuffer* screenBuff, Filter* filter) {
	state->reset();
	if (!deferred) deferred = render->findShader("deferred");

	state->enableCull = false;
	state->enableDepthTest = false;
	state->enableAlphaTest = false;
	state->pass = 5;
	state->shader = deferred;
	state->shadow = shadow;
	state->light = lightDir;

	uint baseSlot = screenBuff->colorBuffers.size() + 1; // Color buffers + Depth buffer
	render->useTexture(TEXTURE_2D, baseSlot, nearBuffer->getColorBuffer(0)->id);
	render->useTexture(TEXTURE_2D, baseSlot + 1, midBuffer->getColorBuffer(0)->id);
	render->useTexture(TEXTURE_2D, baseSlot + 2, farBuffer->getColorBuffer(0)->id);
	filter->draw(scene->mainCamera, render, state, screenBuff->colorBuffers, screenBuff->depthBuffer);
	render->finishDraw();
}

void RenderManager::drawBoundings(Render* render, RenderState* state, Scene* scene, Camera* camera) {
	for (unsigned int i = 0; i<scene->boundingNodes.size(); i++) {
		Node* node = scene->boundingNodes[i];
		render->draw(camera, node->drawcall, state);
	}
}

void RenderManager::enableShadow(Render* render) {
	if (!deferred) deferred = render->findShader("deferred");
	useShadow = true;
	render->useShader(deferred);
	deferred->setInt("useShadow", 1);
}

void RenderManager::disableShadow(Render* render) {
	if (!deferred) deferred = render->findShader("deferred");
	useShadow = false;
	render->useShader(deferred);
	deferred->setInt("useShadow", 0);
}

void RenderManager::showBounding() {
	drawBounding = true;
}

void RenderManager::hideBounding() {
	drawBounding = false;
}

