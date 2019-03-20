/*
 * shadermanager.h
 *
 *  Created on: 2017-4-7
 *      Author: a
 */

#ifndef SHADERMANAGER_H_
#define SHADERMANAGER_H_

#include "shader.h"

class ShaderManager {
private:
	std::map<std::string,Shader*> shaders;
public:
	ShaderManager();
	~ShaderManager();
	Shader* addShader(const char* name, const char* vs, const char* fs);
	Shader* addShader(const char* name, const char* vs, const char* fs, const char* gs);
	Shader* findShader(const char* name);
};


#endif /* SHADERMANAGER_H_ */
