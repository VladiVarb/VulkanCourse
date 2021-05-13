#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "utilities.h"

struct UboModelStruct {
	glm::mat4 model;
};

struct PushModel{
	glm::mat4 model;
};

class Mesh
{
public:
	UboModelStruct strUboModel;
	PushModel pushModel;

	Mesh();
	~Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice,
		VkQueue transferQueue, VkCommandPool transferCommandPool,
		std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
		int newTextureId); //vertices -> vector of all the vertices we want to draw with the device
	
	void Mesh::calcNormal(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	void setModelMatrix(glm::mat4 newModelMatrix);
	PushModel getPushConstModelStr();


	int getVerticesCount(); //needed to pass as param in recordCommads->vkCmdDraw
	VkBuffer* getVertexBuffer();

	int getIndcesCount();
	VkBuffer getIndexBuffer();

	
	int getTexID();

	void destroyVertexBuffer();
	void destroyIndexBuffer();
	//uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertiesWeWant);
	
private: 
	//UboModelStruct uboModel;
	
	int verticesCount;
	int indexCount;
	int texId;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice  logicalDevice;

	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t> *indices);	

};

