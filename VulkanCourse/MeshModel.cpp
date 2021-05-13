#include "MeshModel.h"



MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> meshes)
{
	meshList = meshes;
	modelMatrix = glm::mat4(1.0f);
}


Mesh * MeshModel::getMesh(int ind)
{
	return &meshList[ind];
}

void MeshModel::setMesh(int i, Mesh mesh)
{
	meshList[i] = mesh;
}

int MeshModel::getModelID()
{
	return modelID;
}

void MeshModel::setModelID(int newID)
{
	modelID = newID;
	
}

glm::mat4 MeshModel::getModelMatrix()
{
	return modelMatrix;
}

int MeshModel::getMeshListSize()
{
	return meshList.size();
}

MeshModel::~MeshModel()
{
}

void MeshModel::destroyMeshModel()
{
	for (size_t i = 0; i < meshList.size(); ++i)
	{
		meshList[i].destroyIndexBuffer();
		meshList[i].destroyVertexBuffer();
	}
}

modelMaterials MeshModel::LoadTexNamesForMats(const aiScene* scene)
{
	modelMaterials tmpModelMaterial;
	tmpModelMaterial.modelTexturesFileNames.resize(scene->mNumMaterials);
	tmpModelMaterial.modelTexturesIDs.resize(scene->mNumMaterials);								//will be updated at later point

	aiString  path;
	std::string fileName;
	aiMaterial* tmpMaterial;
	aiColor4D color;
	aiColor4D alfa;
	glm::vec4 vec4color;
	for (int i = 0; i < scene->mNumMaterials; i++)
	{	

		
		
		tmpMaterial = scene->mMaterials[i];

		aiGetMaterialColor(tmpMaterial, AI_MATKEY_COLOR_DIFFUSE, &color);
	//	aiGetMaterialColor(tmpMaterial, AI_MATKEY_COLOR_TRANSPARENT, &alfa);

		tmpModelMaterial.materialName = tmpMaterial->GetName().data;
		vec4color.r = color.r;
		vec4color.g = color.g;
		vec4color.b = color.b;
	//	vec4color.a = alfa.a;
		tmpModelMaterial.colorMaterials.push_back(vec4color);
		if (aiGetMaterialTextureCount(tmpMaterial, aiTextureType_DIFFUSE)
			&& (AI_SUCCESS == aiGetMaterialTexture(tmpMaterial, aiTextureType_DIFFUSE, 0, &path))) //0-> get the first texture 
		{
			fileName = std::string(path.data);
			int idx = fileName.rfind("\\");
			//tmpStrTextures[i] = fileName.substr(idx + 1);
			tmpModelMaterial.modelTexturesFileNames[i] = fileName.substr(idx + 1);
			tmpModelMaterial.availableTexturesCount++;
		}
		
		else
		{
			tmpModelMaterial.modelTexturesFileNames[i] = "blank_tex.jpg";
		}
	}

	return tmpModelMaterial;
}

std::vector<Mesh> MeshModel::LoadNode(VkPhysicalDevice physicalDevice, VkDevice logicalDevice,  //TODO : make it with list of lists train recursion like a boss
										VkQueue transferQueue, 
										VkCommandPool commandPool, 
										const aiScene * scene, 
										aiNode * node,
										std::vector<int>& texNamesToIDs,
										modelMaterials &materials)
{
	std::vector<Mesh> majorMeshList;
	for(size_t i = 0; i < node->mNumMeshes; i++)
	{	
		//Load meshes in current node into majorMeshList
		Mesh tmpMesh = 	MeshModel::LoadMesh(physicalDevice, logicalDevice,
							transferQueue, commandPool,
							scene, 	scene->mMeshes[node->mMeshes[i]],
							texNamesToIDs,
							materials);

		majorMeshList.push_back(tmpMesh);
	}

	//then load meshes from other nodes
	for(size_t i = 0; i < node->mNumChildren; i++)
	{
		std::vector<Mesh> minorMeshList = MeshModel::LoadNode(physicalDevice, logicalDevice,  //TODO : make it with list of lists train recursion like a boss
			transferQueue,
			commandPool,
			scene,
			node->mChildren[i],
			texNamesToIDs,
			materials);
		majorMeshList.insert(majorMeshList.end(), minorMeshList.begin(), minorMeshList.end());

	}
	
	return majorMeshList;
}


Mesh MeshModel::LoadMesh(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, 
						 VkQueue transferQueue, VkCommandPool commandPool, 
						 const aiScene * scene, aiMesh * mesh, std::vector<int>& texNamesToIDs,modelMaterials& materials)
{
	
	std::vector<Vertex> tmpVertex(mesh->mNumVertices);
	std::vector<uint32_t> indices;// (mesh->mNumFaces);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{	
		tmpVertex[i].pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (glm::vec3(materials.colorMaterials[mesh->mMaterialIndex]) == glm::vec3(0.f))
		{
			tmpVertex[i].col = glm::vec3(1.0f);
		}
		else
		{
			tmpVertex[i].col = glm::vec3(materials.colorMaterials[mesh->mMaterialIndex]);//glm::vec3(1.0f);
		}
		
		//if(mesh->HasTextureCoords(i))
		//{
		tmpVertex[i].tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		//}
		//else
		//{
		//	tmpVertex[i].tex = { 0.0f, 0.0f };
		//}

	
	}
	//for every poligon[i](face) get the indices of vertices[j]
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}
	
	
	//calc normals in mesh creation
	Mesh tmpMesh (physicalDevice, logicalDevice,
					transferQueue, commandPool,
					&tmpVertex, &indices, static_cast<uint32_t>(texNamesToIDs[mesh->mMaterialIndex]));

	
	return tmpMesh;
}