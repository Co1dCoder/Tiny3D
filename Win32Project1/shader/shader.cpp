#include "shader.h"
#include "../constants/constants.h"
using namespace std;

//#define DEBUG_SHADER 1

Shader::Shader(const char* vert, const char* frag, const char* tesc, const char* tese, const char* geom) {
	vertName = vert, fragName = frag;
	program = new ShaderProgram(vert, frag, tesc, tese, geom);
	bindedTexs.clear();
	texSlots.clear();
}

Shader::~Shader() {
	delete program;
	program = NULL;
	paramLocations.clear();
	attribLocations.clear();
	bindedTexs.clear();
	texSlots.clear();
}

void Shader::attachDef(const char* def, const char* value) {
	program->attachDef(def, value);
}

void Shader::attachEx(string ex) {
	program->attachEx(ex.data());
}

void Shader::compose() {
	program->compose();
}

void Shader::compile(bool preload) {
	program->compile(preload);
}

void Shader::dettach() {
	program->dettach();
}

void Shader::use() {
	program->use();
}

void Shader::addAttrib(const char* name) {
	GLint location=glGetAttribLocation(program->shaderProg,name);
	if ((int)location != INVALID_LOCATION)
		attribLocations.insert(pair<string, GLint>(name, location));
}

void Shader::addParam(const char* name) {
	GLint location=glGetUniformLocation(program->shaderProg,name);
	if ((int)location != INVALID_LOCATION)
		paramLocations.insert(pair<string, GLint>(name, location));
}

int Shader::findAttribLocation(const char* attrib) {
	map<string,GLint>::iterator itor=attribLocations.find(attrib);
	if(itor!=attribLocations.end())
		return itor->second;
	return INVALID_LOCATION;
}

int Shader::findParamLocation(const char* param) {
	map<string, GLint>::iterator itor=paramLocations.find(param);
	if(itor!=paramLocations.end())
		return itor->second;
	return INVALID_LOCATION;
}

bool Shader::getError(const char* param, int location) {
#ifdef DEBUG_SHADER
	GLenum error = glGetError();
	if (error == GL_NO_ERROR) return false;
	else {
		printf("%s,%s param error %d: %s,%d\n", vertName.data(), fragName.data(), error, param, location);
		return true;
	}
#endif
	return false;
}

void Shader::setInt(const char* param,int value) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform1i(program->shaderProg, location, value);
	if (getError(param, location))
		printf("value is: %d\n", value);
}

void Shader::setSampler(const char* param,int value) {
	setInt(param,value);
}

void Shader::setFloat(const char* param,float value) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform1f(program->shaderProg, location, value);
	if (getError(param, location))
		printf("value is: %f\n", value);
}

void Shader::setVector2(const char* param,float x,float y) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform2f(program->shaderProg, location, x, y);
	getError(param, location);
}

void Shader::setVector3(const char* param,float x,float y,float z) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform3f(program->shaderProg, location, x, y, z);
	if (getError(param, location))
		printf("vec3 is: %f,%f,%f\n", x, y, z);
}

void Shader::setVector4(const char* param,float x,float y,float z,float w) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform4f(program->shaderProg, location, x, y, z, w);
	getError(param, location);
}

void Shader::setVector2v(const char* param, float* arr) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform2fv(program->shaderProg, location, 1, arr);
	getError(param, location);
}

void Shader::setVector3v(const char* param, float* arr) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform3fv(program->shaderProg, location, 1, arr);
	if (getError(param, location)) 
		printf("vec3 is: %f,%f,%f\n", arr[0], arr[1], arr[2]);
}

void Shader::setVector4v(const char* param, float* arr) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniform4fv(program->shaderProg, location, 1, arr);
	getError(param, location);
}

void Shader::setMatrix4(const char* param,float* matrix) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniformMatrix4fv(program->shaderProg, location, 1, GL_FALSE, matrix);
	if (getError(param, location)) {
		printf("matrix is: \n");
		for (int i = 0; i < 4; i++)
			printf("%f %f %f %f\n", matrix[i * 4 + 0], matrix[i * 4 + 1], matrix[i * 4 + 2], matrix[i * 4 + 3]);
	}
}

void Shader::setMatrix4(const char* param,int count,float* matrices) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniformMatrix4fv(program->shaderProg, location, count, GL_FALSE, matrices);
	getError(param, location);
}

void Shader::setMatrix3x4(const char* param, int count, float* matrices) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniformMatrix3x4fv(program->shaderProg, location, count, GL_FALSE, matrices);
	getError(param, location);
}

void Shader::setMatrix3(const char* param, float* matrix) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniformMatrix3fv(program->shaderProg, location, 1, GL_FALSE, matrix);
	getError(param, location);
}

void Shader::setMatrix3(const char* param, int count, float* matrices) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program)
		glProgramUniformMatrix3fv(program->shaderProg, location, count, GL_FALSE, matrices);
	getError(param, location);
}

void Shader::setHandle64(const char* param, u64 value) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program) {
		glProgramUniformHandleui64ARB(program->shaderProg, location, value);
		bindedTexs[value] = true;
	}
	if (getError(param, location))
		printf("value is: %lld\n", value);
}

void Shader::setHandle64v(const char* param, int count, u64* arr) {
	int location = findParamLocation(param);
	if (location == INVALID_LOCATION) {
		addParam(param);
		location = findParamLocation(param);
	}
	if (location != INVALID_LOCATION && program) {
		glProgramUniformHandleui64vARB(program->shaderProg, location, count, arr);
		for (int i = 0; i < count; i++)
			bindedTexs[arr[i]] = true;
	}
	if (getError(param, location))
		printf("value is: %lld\n", arr[count - 1]);
}

