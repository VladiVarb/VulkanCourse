#pragma once

#include "Mesh.h"

#include <glm\glm.hpp>

#include <assimp/material.h>
#include <assimp/scene.h>

//#include <vector>

class MeshModel
{
public:
	MeshModel();
	MeshModel(std::vector<Mesh> meshes);
	
	Mesh* getMesh(int ind);
	void setMesh(int i, Mesh mesh);
	
	int getModelID();
	void setModelID(int newID);

	glm::mat4 getModelMatrix();
	int getMeshListSize();

	~MeshModel();
	void destroyMeshModel();
	static modelMaterials LoadTexNamesForMats(const aiScene * scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
		VkQueue transferQueue, /*used for staging buffer for texture*/
		VkCommandPool commandPool, //Why need command pool
		const aiScene * scene,
		aiNode * node,
		std::vector<int>& texNamesToIDs,
		modelMaterials& materials);
	static Mesh LoadMesh(VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VkQueue transferQueue,
 /*used for staging buffer for texture*/
		VkCommandPool commandPool, const aiScene * scene,
		aiMesh * mesh,
		std::vector<int>& texNamesToIDs,
		modelMaterials &materials);

	std::vector<std::string> objectsNames;
	glm::vec3 pos = glm::vec3(0.f,0.f,0.f);
	float angle = 0.f;


private:
	std::vector<Mesh> meshList;
	glm::mat4 modelMatrix;
	int modelID;

};

