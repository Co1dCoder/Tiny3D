#include "staticDrawcall.h"

StaticDrawcall::StaticDrawcall() {}

StaticDrawcall::StaticDrawcall(Batch* batch) :Drawcall() {
	vbos = new GLuint[8];
	vertexCount = batch->vertexCount;
	indexCount = batch->indexCount;
	indexed = indexCount > 0 ? true : false;
	this->batch = batch;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(8, vbos);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[VERTEX_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(GLfloat),
		batch->vertexBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[NORMAL_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(GLfloat),
		batch->normalBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(NORMAL_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[TEXCOORD_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(GLfloat),
		batch->texcoordBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(TEXCOORD_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[COLOR_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(GLubyte),
		batch->colorBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(COLOR_LOCATION, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOR_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[TEXTUREID_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * batch->textureChannel *sizeof(short),
		batch->textureIdBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(TEXTURE_LOCATION, batch->textureChannel, GL_SHORT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(TEXTURE_LOCATION);


	glBindBuffer(GL_ARRAY_BUFFER, vbos[MODEL_MATRIX_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 12 * sizeof(GLfloat),
		batch->modelMatrices, GL_DYNAMIC_DRAW);
	for (int i = 0; i < 3; i++) {
		glVertexAttribPointer(MODEL_MATRIX_LOCATION + i, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 12,
			(void*)(sizeof(float)*i * 4));
		glEnableVertexAttribArray(MODEL_MATRIX_LOCATION + i);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbos[NORMAL_MATRIX_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 9 * sizeof(GLfloat),
		batch->normalMatrices, GL_DYNAMIC_DRAW);
	for (int i = 0; i < 3; i++) {
		glVertexAttribPointer(NORMAL_MATRIX_LOCATION + i, 3, GL_FLOAT, GL_FALSE, sizeof(float)* 9,
			(void*)(sizeof(float)*i * 3));
		glEnableVertexAttribArray(NORMAL_MATRIX_LOCATION + i);
	}

	if (indexed) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[INDEX_VBO]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(GLuint),
			batch->indexBuffer, GL_STATIC_DRAW);
	}

	createSimple();
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

StaticDrawcall::~StaticDrawcall() {
	releaseSimple();
	glDeleteBuffers(8,vbos);
	delete[] vbos; vbos=NULL;
	glDeleteVertexArrays(1,&vao);
}

void StaticDrawcall::createSimple() {
	vboSimple = new GLuint[5];

	glGenVertexArrays(1, &vaoSimple);
	glBindVertexArray(vaoSimple);
	glGenBuffers(5, vboSimple);

	glBindBuffer(GL_ARRAY_BUFFER, vboSimple[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(GLfloat),
		batch->vertexBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vboSimple[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(GLfloat),
		batch->texcoordBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vboSimple[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * batch->textureChannel *sizeof(short),
		batch->textureIdBuffer, GL_STATIC_DRAW);
	glVertexAttribPointer(2, batch->textureChannel, GL_SHORT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vboSimple[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 12 * sizeof(GLfloat),
		batch->modelMatrices, GL_DYNAMIC_DRAW);
	for (int i = 0; i < 3; i++) {
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 12,
			(void*)(sizeof(float)*i * 4));
		glEnableVertexAttribArray(3 + i);
	}

	if (indexed) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSimple[4]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(GLuint),
			batch->indexBuffer, GL_STATIC_DRAW);
	}
}

void StaticDrawcall::releaseSimple() {
	glDeleteBuffers(5, vboSimple);
	delete[] vboSimple; vboSimple = NULL;
	glDeleteVertexArrays(1, &vaoSimple);
}

void StaticDrawcall::draw(Shader* shader,bool simple) {
	if (!simple)
		glBindVertexArray(vao);
	else
		glBindVertexArray(vaoSimple);
	if(!indexed)
		glDrawArrays(GL_TRIANGLES,0,vertexCount);
	else
		glDrawElements(GL_TRIANGLES,indexCount,GL_UNSIGNED_INT,0);
}

void StaticDrawcall::updateMatrices(Batch* batch, bool updateNormals) {
	glBindBuffer(GL_ARRAY_BUFFER, vbos[MODEL_MATRIX_VBO]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 12 * sizeof(GLfloat),
		batch->modelMatrices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboSimple[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 12 * sizeof(GLfloat),
		batch->modelMatrices, GL_DYNAMIC_DRAW);

	if (updateNormals) {
		glBindBuffer(GL_ARRAY_BUFFER, vbos[NORMAL_MATRIX_VBO]);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 9 * sizeof(GLfloat),
			batch->normalMatrices, GL_DYNAMIC_DRAW);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
