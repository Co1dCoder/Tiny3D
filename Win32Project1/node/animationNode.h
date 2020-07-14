/*
 * animationNode.h
 *
 *  Created on: 2017-6-24
 *      Author: a
 */

#ifndef ANIMATIONNODE_H_
#define ANIMATIONNODE_H_

#include "node.h"
#include "../object/animationObject.h"

class AnimationNode: public Node {
private:
	Animation* animation;
	bool needUpdateAnimNode;
public:
	AnimationNode(const vec3& boundingSize);
	virtual ~AnimationNode();
	virtual void prepareDrawcall() { needCreateDrawcall = false; }
	virtual void updateRenderData() {}
	virtual void updateDrawcall() { needUpdateDrawcall = false; }
	void setAnimation(Scene* scene, Animation* anim);
	AnimationObject* getObject();
	void animate(float velocity);
	virtual void translateNode(Scene* scene, float x, float y, float z); // Local translate
	void translateNodeAtWorld(Scene* scene, float x, float y, float z); // Global translate (used in collision feedback)
	void translateNodeCenterAtWorld(Scene* scene, float x, float y, float z);
	void rotateNodeObject(Scene* scene, float ax, float ay, float az);
	void scaleNodeObject(Scene* scene, float sx, float sy, float sz);
	void setUpdate(bool need) { needUpdateAnimNode = need; }
	virtual void pushToUpdate(Scene* scene);
};


#endif /* ANIMATIONNODE_H_ */
