#ifndef ASSETMANAGER_H_
#define ASSETMANAGER_H_

#define COMMON_TEXTURE "texture/common"

#include "../mesh/mesh.h"
#include "../animation/animation.h"
#include "../texture/imageset.h"
#include "../texture/cubemap.h"

class AssetManager {
public:
	static AssetManager* assetManager;
public:
	static void Init();
	static void Release();
public:
	std::map<std::string, Mesh*> meshes;
	std::map<std::string, Animation*> animations;
	ImageSet* textures;
	CubeMap* skyTexture;
private:
	AssetManager();
	~AssetManager();
public:
	void addMesh(const char* name, Mesh* mesh);
	void addAnimation(const char* name, Animation* animation);
	void addTexture(const char* name);
	void initTextureArray();
	int findTexture(const char* name);
	void setSkyTexture(CubeMap* tex);
	CubeMap* getSkyTexture();
};

#endif