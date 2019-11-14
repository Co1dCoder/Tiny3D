#include "node.h"
#include "../util/util.h"
#include "../instance/instance.h"
#include "../scene/scene.h"

std::vector<Node*> Node::nodesToUpdate;
std::vector<Node*> Node::nodesToRemove;

Node::Node(const vec3& position,const vec3& size) {
	this->position.x=position.x;
	this->position.y=position.y;
	this->position.z=position.z;
	this->position.w=1;
	this->size.x=size.x;
	this->size.y=size.y;
	this->size.z=size.z;
	boundingBox=new AABB(position,size.x,size.y,size.z);
	objects.clear();
	objectsBBs.clear();
	drawcall=NULL;
	needUpdateDrawcall = false;
	needCreateDrawcall = false;
	needUpdateNormal = false;
	needUpdateNode = false;
	uTransformMatrix = NULL;
	uNormalMatrix = NULL;

	parent=NULL;
	children.clear();
	nodeBBs.clear();

	type = TYPE_NULL;
	shadowLevel = 3;
	detailLevel = 3;
}

Node::~Node() {
	if (uTransformMatrix) delete uTransformMatrix; 
	uTransformMatrix = NULL;
	if (uNormalMatrix) delete uNormalMatrix; 
	uNormalMatrix = NULL;

	if(boundingBox)
		delete boundingBox;
	boundingBox=NULL;

	objectsBBs.clear();
	for (uint i = 0; i < objects.size(); i++) 
		delete objects[i];
	objects.clear();

	if(drawcall)
		delete drawcall;
	drawcall=NULL;

	nodeBBs.clear();
	clearChildren();
}

void Node::clearChildren() {
	for(unsigned int i=0;i<children.size();i++) {
		Node* child=children[i];
		child->clearChildren();
		delete child;
	}
	children.clear();
}

bool Node::checkInCamera(Camera* camera) {
	return checkInFrustum(camera->frustum);
}

bool Node::checkInFrustum(Frustum* frustum) {
	if (boundingBox)
		return boundingBox->checkWithCamera(frustum, detailLevel);
	return true;
}

// Update Object's bounding box from local to world
void Node::updateObjectBoundingInNode(Object* object) {
	BoundingBox* objectBB = object->bounding;
	if (objectBB) {
		mat4 nodeMat; nodeMat.LoadIdentity();
		recursiveTransform(nodeMat);
		vec4 localBB4(object->localBoundPosition.x, object->localBoundPosition.y, object->localBoundPosition.z, 1.0);
		vec4 bb4 = nodeMat * localBB4;
		objectBB->update(vec3(bb4.x / bb4.w, bb4.y / bb4.w, bb4.z / bb4.w));
	}
}

void Node::addObject(Scene* scene, Object* object) {
	objects.push_back(object);
	object->caculateLocalAABB(false, false);
	BoundingBox* objectBB = object->bounding;
	if (objectBB) {
		updateObjectBoundingInNode(object);
		objectsBBs.push_back(objectBB);
		boundingBox->merge(objectsBBs);

		Node* superior = parent;
		while (superior) {
			superior->updateBounding();
			superior = superior->parent;
		}
	}
	needCreateDrawcall = true;
	pushToUpdate();
}

Object* Node::removeObject(Object* object) {
	std::vector<Object*>::iterator it;
	std::vector<BoundingBox*>::iterator itbb;
	for (it = objects.begin(); it != objects.end(); ++it) {
		if ((*it) == object) {
			objects.erase(it);
			for (itbb = objectsBBs.begin(); itbb != objectsBBs.end(); ++itbb) {
				if ((*itbb) == object->bounding) {
					objectsBBs.erase(itbb);
					break;
				}
			}
			boundingBox->merge(objectsBBs);

			Node* superior = parent;
			while (superior) {
				superior->updateBounding();
				superior = superior->parent;
			}

			needCreateDrawcall = true;
			pushToUpdate();

			return object;
		}
	}
	return NULL;
}

// Update the Node's bounding with objects maybe its children's
void Node::updateBaseNodeBounding() {
	for(unsigned int i=0;i<objects.size();i++)
		updateObjectBoundingInNode(objects[i]);
	if (objects.size()>0) {
		if (objectsBBs.size() > 0) 
			boundingBox->merge(objectsBBs);
		else if (objectsBBs.size() <= 0) { // Base Node and without object boundings
			mat4 nodeTransform; nodeTransform.LoadIdentity();
			recursiveTransform(nodeTransform);
			boundingBox->update(nodeTransform * vec4(0, 0, 0, 1));
		}
	}

	for(unsigned int n=0;n<children.size();n++)
		children[n]->updateBaseNodeBounding();
}

// Update Node's bounding & its children's & children's children...
void Node::updateSelfAndDownwardNodesBounding() {
	nodeBBs.clear();
	for(unsigned int n=0;n<children.size();n++) {
		Node* child=children[n];
		child->updateSelfAndDownwardNodesBounding();
		AABB* childAABB=(AABB*)child->boundingBox;
		if(childAABB&&(childAABB->sizex>0||childAABB->sizey>0||childAABB->sizez>0))
			nodeBBs.push_back(child->boundingBox);
	}
	if (nodeBBs.size()>0)
		boundingBox->merge(nodeBBs);
}

// Move Node's objects's bounding
void Node::moveBaseObjectsBounding(float dx, float dy, float dz) {
	vec3 offset = vec3(dx, dy, dz);
	for (uint i = 0; i < objects.size(); i++) {
		BoundingBox* objectBB = objects[i]->bounding;
		if (objectBB) objectBB->update(objectBB->position + offset);
	}
	for (uint n = 0; n < children.size(); n++)
		children[n]->moveBaseObjectsBounding(dx, dy, dz);
}

// Just move Node's bounding & its children's & children's children...
void Node::moveSelfAndDownwardNodesBounding(float dx, float dy, float dz) {
	if (boundingBox) {
		vec3 offset = vec3(dx, dy, dz);
		boundingBox->update(boundingBox->position + offset);
	}
	for (uint n = 0; n < children.size(); n++) 
		children[n]->moveSelfAndDownwardNodesBounding(dx, dy, dz);
}

// Update current Node's bounding
void Node::updateBounding() {
	nodeBBs.clear();
	for (unsigned int n = 0; n<children.size(); n++) {
		Node* child = children[n];
		AABB* childAABB = (AABB*)child->boundingBox;
		if (childAABB && (childAABB->sizex > 0 || childAABB->sizey > 0 || childAABB->sizez > 0))
			nodeBBs.push_back(child->boundingBox);
	}
	if (nodeBBs.size()>0)
		boundingBox->merge(nodeBBs);
}

// Update Node's drawcall & its children's & children's children...
void Node::updateSelfAndDownwardNodesDrawcall(bool updateNormal) {
	if (objects.size() > 0) {
		needUpdateNormal = updateNormal;
		needUpdateDrawcall = true;
		pushToUpdate();
	}

	for (unsigned int i = 0; i < children.size(); i++)
		children[i]->updateSelfAndDownwardNodesDrawcall(updateNormal);
}

// Find ancestor of this Node
Node* Node::getAncestor() {
	Node* root = this;
	Node* superior = parent;
	while (superior) {
		root = superior;
		superior = superior->parent;
	}
	return root;
}

void Node::attachChild(Node* child) {
	children.push_back(child);
	child->parent=this;

	child->updateBaseNodeBounding();
	child->updateSelfAndDownwardNodesBounding();

	Node* superior = this;
	while (superior) {
		superior->updateBounding();
		superior = superior->parent;
	}

	updateSelfAndDownwardNodesDrawcall(false);
}

Node* Node::detachChild(Node* child) {
	std::vector<Node*>::iterator it;
	for(it=children.begin();it!=children.end();++it) {
		if((*it)==child) {
			child->parent=NULL;
			children.erase(it);

			Node* superior = this;
			while (superior) {
				superior->updateBounding();
				superior = superior->parent;
			}

			if (child->type == TYPE_INSTANCE) {
				for (uint i = 0; i < child->objects.size(); i++) {
					Object* object = child->objects[i];
					Instance::instanceTable[object->mesh]--;
					if (object->meshMid)
						Instance::instanceTable[object->meshMid]--;
					if (object->meshLow)
						Instance::instanceTable[object->meshLow]--;
				}
			}

			return child;
		}
	}
	return NULL;
}

void Node::translateNode(float x, float y, float z) {
	float dx = x - position.x;
	float dy = y - position.y;
	float dz = z - position.z;

	position.x = x;
	position.y = y;
	position.z = z;

	// Inefficient
	//updateBaseNodeBounding();
	//updateSelfAndDownwardNodesBounding();
	moveBaseObjectsBounding(dx, dy, dz);
	moveSelfAndDownwardNodesBounding(dx, dy, dz);

	Node* superior = parent;
	while (superior) {
		superior->updateBounding();
		superior = superior->parent;
	}

	updateSelfAndDownwardNodesDrawcall(false);
}

void Node::translateNodeObject(int i, float x, float y, float z) {
	Object* object = objects[i];
	object->setPosition(x, y, z);
	object->caculateLocalAABB(false, false);

	updateObjectBoundingInNode(object);
	boundingBox->merge(objectsBBs);
	Node* superior = parent;
	while (superior) {
		superior->updateBounding();
		superior = superior->parent;
	}
	needUpdateNormal = false;
	needUpdateDrawcall = true;
	pushToUpdate();
}

void Node::translateNodeObjectCenterAtWorld(int i, float x, float y, float z) {
	Object* object = objects[i];
	vec3 worldCenter = object->bounding->position;
	vec3 offset = vec3(x, y, z) - worldCenter;
	vec3 localPosition = object->position;
	translateNodeObject(i, localPosition.x + offset.x, localPosition.y + offset.y, localPosition.z + offset.z);
}

void Node::rotateNodeObject(int i, float ax, float ay, float az) {
	Object* object = objects[i];
	object->setRotation(ax,ay,az);
	object->caculateLocalAABB(false, false);

	updateObjectBoundingInNode(object);
	boundingBox->merge(objectsBBs);
	Node* superior = parent;
	while (superior) {
		superior->updateBounding();
		superior = superior->parent;
	}
	needUpdateNormal = true;
	needUpdateDrawcall = true;
	pushToUpdate();
}

void Node::scaleNodeObject(int i, float sx, float sy, float sz) {
	Object* object = objects[i];
	object->setSize(sx, sy, sz);
	object->caculateLocalAABB(false, false);

	updateObjectBoundingInNode(object);
	boundingBox->merge(objectsBBs);
	Node* superior = parent;
	while (superior) {
		superior->updateBounding();
		superior = superior->parent;
	}
	if (sx == sy && sy == sz) needUpdateNormal = false;
	else needUpdateNormal = true;
	needUpdateDrawcall = true;
	pushToUpdate();
}

void Node::pushToUpdate() {
	if (!needUpdateNode) {
		Node::nodesToUpdate.push_back(this);
		needUpdateNode = true;
	}
}

void Node::updateNode() {
	if (type != TYPE_ANIMATE) {
		recursiveTransform(nodeTransform);
		for (unsigned int i = 0; i < objects.size(); i++) {
			Object* object = objects[i];
			object->transformMatrix = nodeTransform * object->localTransformMatrix;
			object->transformTransposed = object->transformMatrix.GetTranspose();
			object->rotateQuat = MatrixToQuat(object->rotateMat);
			AABB* bbox = (AABB*)object->bounding;
			object->boundInfo = vec4(bbox->sizex, bbox->sizey, bbox->sizez, bbox->position.y);
			if (object->transforms) {
				object->transforms[0] = object->transformMatrix.entries[12];
				object->transforms[1] = object->transformMatrix.entries[13];
				object->transforms[2] = object->transformMatrix.entries[14];
				object->transforms[3] = object->size.x;
			}
			if (object->transformsFull) {
				object->transformsFull[0] = (object->transforms[0]);
				object->transformsFull[1] = (object->transforms[1]);
				object->transformsFull[2] = (object->transforms[2]);
				object->transformsFull[3] = (object->transforms[3]);
				object->transformsFull[4] = (object->rotateQuat.x);
				object->transformsFull[5] = (object->rotateQuat.y);
				object->transformsFull[6] = (object->rotateQuat.z);
				object->transformsFull[7] = (object->rotateQuat.w);
				object->transformsFull[8] = (object->boundInfo.x);
				object->transformsFull[9] = (object->boundInfo.y);
				object->transformsFull[10] = (object->boundInfo.z);
				object->transformsFull[11] = (object->boundInfo.w);
			}
		}
	}
	needUpdateNode = false;
}

void Node::pushToRemove() {
	Node::nodesToRemove.push_back(this);
}

void Node::recursiveTransform(mat4& finalNodeMatrix) {
	if(parent) {
		mat4 parentTransform;
		parent->recursiveTransform(parentTransform);
		finalNodeMatrix=parentTransform*translate(position.x,position.y,position.z);
	} else
		finalNodeMatrix=translate(position.x,position.y,position.z);
}
