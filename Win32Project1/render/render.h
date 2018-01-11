/*
 * render.h
 *
 *  Created on: 2017-4-7
 *      Author: a
 */

#ifndef RENDER_H_
#define RENDER_H_

#include "../shader/shadermanager.h"
#include "../camera/camera.h"
#include "../framebuffer/framebuffer.h"
#include "drawcall.h"

class Render {
private:
	void initEnvironment();
public: // Global render state
	bool enableCull;
	int cullMode;
	bool drawLine;
	bool enableDepthTest;
	int depthTestMode;
	COLOR clearColor;
	Shader* currentShader;
public:
	int viewWidth,viewHeight;
	ShaderManager* shaders;
public:
	Render();
	~Render();
	void clearFrame(bool clearColor,bool clearDepth,bool clearStencil);
	void setState(const RenderState* state);
	void setDepthTest(bool enable,int testMode);
	void setCullState(bool enable);
	void setCullMode(int mode);
	void setDrawLine(bool line);
	void setClearColor(float r,float g,float b,float a);
	void setViewPort(int width,int height);
	void resize(int width,int height,Camera* mainCamera);
	Shader* findShader(const char* shader);
	void useShader(Shader* shader);
	void draw(Camera* camera, Drawcall* drawcall, const RenderState* state);
	void finishDraw();
	void setFrameBuffer(FrameBuffer* framebuffer);
};


#endif /* RENDER_H_ */
