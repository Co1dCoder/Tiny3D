/*
 * animationDrawcall.h
 *
 *  Created on: 2017-6-24
 *      Author: a
 */

#ifndef ANIMATIONDRAWCALL_H_
#define ANIMATIONDRAWCALL_H_

#include "drawcall.h"
#include "../animation/animation.h"

#define UNIFORM_BONEMATS "boneMats"

class AnimationDrawcall: public Drawcall {
private:
	float* vertices;
	float* normals;
	float* texcoords;
	unsigned char* colors;
	unsigned char* boneids;
	float* weights;
	unsigned short* indices;
	int indexCount;
	int textureChannel;

	Animation* animation;
	int boneCount;
public:
	AnimationDrawcall();
	AnimationDrawcall(Animation* anim);
	virtual ~AnimationDrawcall();
	virtual void createSimple();
	virtual void releaseSimple();
	virtual void draw(Shader* shader,bool simple);
};

#endif /* ANIMATIONDRAWCALL_H_ */
