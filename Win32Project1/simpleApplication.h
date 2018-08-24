#ifndef SIMPLE_APPLICATION_H_
#define SIMPLE_APPLICATION_H_

#include "application/application.h"
#include "filter/filter.h"

class SimpleApplication : public Application {
private:
	FrameBuffer* screen;
	Filter* screenFilter;
	Filter* aaFilter;
	Filter* blurFilter;
	Filter* dofFilter;
	std::vector<Texture2D*> dofInput;
public:
	SimpleApplication();
	virtual ~SimpleApplication();
public:
	virtual void init();
	virtual void draw();
	virtual void act(long startTime, long currentTime);
	virtual void moveKey();
	virtual void resize(int width, int height);
	virtual void keyDown(int key);
	virtual void keyUp(int key);
private:
	void initScene();
	void initScreen();
};

#endif