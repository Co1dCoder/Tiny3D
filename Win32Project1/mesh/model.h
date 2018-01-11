/*
 * model.h
 *
 *  Created on: 2017-4-25
 *      Author: a
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "mesh.h"
#include "../model/objloader.h"

class Model: public Mesh {
private:
	ObjLoader* loader;
	void initFaces();
	void initFacesWidthIndices();
public:
	Model();
	Model(const char* obj, const char* mtl, int vt);
	Model(const Model& rhs);
	virtual ~Model();
	void loadModel(const char* obj,const char* mtl,int vt);
	void loadModelSimple(const char* obj,const char* mtl,int vt);
};

#endif /* MODEL_H_ */
