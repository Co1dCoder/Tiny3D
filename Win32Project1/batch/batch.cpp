#include "batch.h"
#include "../constants/constants.h"
#include "../material/materialManager.h"

Batch::Batch() {
	vertexCount=0;
	indexCount=0;
	storeVertexCount=0;
	storeIndexCount=0;
	vertexBuffer=NULL;
	normalBuffer=NULL;
	texcoordBuffer=NULL;
	colorBuffer = NULL;
	objectidBuffer = NULL;
	indexBuffer=NULL;

	fullStatic = false;
	objectCount = 0;
	modelMatrices = NULL;
	normalMatrices = NULL;
}

Batch::~Batch() {
	if (vertexBuffer) delete[] vertexBuffer; vertexBuffer = NULL;
	if (normalBuffer) delete[] normalBuffer; normalBuffer = NULL;
	if (texcoordBuffer) delete[] texcoordBuffer; texcoordBuffer = NULL;
	if (colorBuffer) delete[] colorBuffer; colorBuffer = NULL;
	if (objectidBuffer) delete[] objectidBuffer; objectidBuffer = NULL;
	if (indexBuffer) delete[] indexBuffer; indexBuffer = NULL;

	if (modelMatrices) delete[] modelMatrices; modelMatrices = NULL;
	if (normalMatrices) delete[] normalMatrices; normalMatrices = NULL;
}

void Batch::initBatchBuffers(int vertices,int indices) {
	vertexCount=vertices;
	vertexBuffer=new float[vertexCount*3];
	normalBuffer=new float[vertexCount*3];
	texcoordBuffer=new float[vertexCount*4];
	colorBuffer = new byte[vertexCount * 3];
	objectidBuffer = new byte[vertexCount];
	storeVertexCount=0;

	indexCount=indices;
	if(indexCount>0)
		indexBuffer=new uint[indexCount];
	storeIndexCount=0;

	modelMatrices = new float[MAX_OBJECT_COUNT * 12];
	normalMatrices = new float[MAX_OBJECT_COUNT * 9];
}

void Batch::pushMeshToBuffers(Mesh* mesh,int mid,bool fullStatic,const MATRIX4X4& transformMatrix,const MATRIX4X4& normalMatrix) {
	this->fullStatic = fullStatic;
	int baseVertex=storeVertexCount;
	int currentObject = objectCount;
	for(int i=0;i<mesh->vertexCount;i++) {
		VECTOR4D vertex=mesh->vertices[i];
		VECTOR4D normal(mesh->normals[i].x,mesh->normals[i].y,mesh->normals[i].z,0);
		VECTOR2D texcoord=mesh->texcoords[i];
		
		Material* mat = NULL;
		if (!mesh->materialids && mid >= 0)
			mat = MaterialManager::materials->find(mid);
		else if (mesh->materialids)
			mat = MaterialManager::materials->find(mesh->materialids[i]);
		if (!mat) mat = MaterialManager::materials->find(0);
		VECTOR3D ambient = mat->ambient;
		VECTOR3D diffuse = mat->diffuse;
		VECTOR3D specular = mat->specular;
		VECTOR4D textures = mat->texture;

		if (!fullStatic) {
			vertexBuffer[storeVertexCount * 3] = vertex.x / vertex.w;
			vertexBuffer[storeVertexCount * 3 + 1] = vertex.y / vertex.w;
			vertexBuffer[storeVertexCount * 3 + 2] = vertex.z / vertex.w;

			normalBuffer[storeVertexCount * 3] = normal.x;
			normalBuffer[storeVertexCount * 3 + 1] = normal.y;
			normalBuffer[storeVertexCount * 3 + 2] = normal.z;
		} else {
			vertex = transformMatrix * vertex;
			vertexBuffer[storeVertexCount * 3] = vertex.x / vertex.w;
			vertexBuffer[storeVertexCount * 3 + 1] = vertex.y / vertex.w;
			vertexBuffer[storeVertexCount * 3 + 2] = vertex.z / vertex.w;

			normal = normalMatrix * normal;
			normalBuffer[storeVertexCount * 3] = normal.x;
			normalBuffer[storeVertexCount * 3 + 1] = normal.y;
			normalBuffer[storeVertexCount * 3 + 2] = normal.z;
		}

		textureChannel = textures.y >= 0 ? 4 : 3;
		texcoordBuffer[storeVertexCount * textureChannel] = texcoord.x;
		texcoordBuffer[storeVertexCount * textureChannel + 1] = texcoord.y;
		texcoordBuffer[storeVertexCount * textureChannel + 2] = textures.x;
		if (textureChannel == 4)
			texcoordBuffer[storeVertexCount * textureChannel + 3] = textures.y;

		colorBuffer[storeVertexCount * 3] = (byte)(ambient.x * 255);
		colorBuffer[storeVertexCount * 3 + 1] = (byte)(diffuse.x * 255);
		colorBuffer[storeVertexCount * 3 + 2] = (byte)(specular.x * 255);

		objectidBuffer[storeVertexCount] = currentObject;

		storeVertexCount++;
	}

	if(mesh->indices) {
		for(int i=0;i<mesh->indexCount;i++) {
			int index=baseVertex+mesh->indices[i];
			indexBuffer[storeIndexCount]=(uint)index;
			storeIndexCount++;
		}
	}

	initMatrix(currentObject, transformMatrix, normalMatrix);
	objectCount++;
}

void Batch::updateMatrices(unsigned short objectId, const MATRIX4X4& transformMatrix, const MATRIX4X4* normalMatrix) {
	MATRIX4X4 transform = transformMatrix;
	transform.Transpose();
	for (int m = 0; m < 12; m++)
		modelMatrices[objectId * 12 + m] = transform.entries[m];
	if (normalMatrix) {
		for (int n = 0; n < 3; n++)
			normalMatrices[objectId * 9 + n] = normalMatrix->entries[n];
		for (int n = 3; n < 6; n++)
			normalMatrices[objectId * 9 + n] = normalMatrix->entries[n + 1];
		for (int n = 6; n < 9; n++)
			normalMatrices[objectId * 9 + n] = normalMatrix->entries[n + 2];
	}
}

void Batch::initMatrix(unsigned short currentObject, const MATRIX4X4& transformMatrix, const MATRIX4X4& normalMatrix) {
	MATRIX4X4 transform = transformMatrix;
	transform.Transpose();
	for (int m = 0; m < 12; m++)
		modelMatrices[currentObject * 12 + m] = transform.entries[m];
	for (int n = 0; n < 3; n++)
		normalMatrices[currentObject * 9 + n] = normalMatrix.entries[n];
	for (int n = 3; n < 6; n++)
		normalMatrices[currentObject * 9 + n] = normalMatrix.entries[n + 1];
	for (int n = 6; n < 9; n++)
		normalMatrices[currentObject * 9 + n] = normalMatrix.entries[n + 2];
}

