#include "Mesh.h"



Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice,
			VkQueue transferQueue, VkCommandPool transferCommandPool,
			std::vector<Vertex>* vertices, std::vector<uint32_t>* indices,
			int newTextureId)
{
	strUboModel.model = glm::mat4(1.0f);
	pushModel.model = glm::mat4(1.0f);
	verticesCount = vertices->size();
	indexCount = indices->size();
	physicalDevice = newPhysicalDevice;
	logicalDevice = newDevice;
	texId = newTextureId;
	calcNormal(*vertices, *indices);
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);	
}

void Mesh::setModelMatrix(glm::mat4 newModel)
{
	pushModel.model = newModel;
}

PushModel Mesh::getPushConstModelStr()
{
	return pushModel;
}

int Mesh::getVerticesCount()
{
	return verticesCount;
}

VkBuffer* Mesh::getVertexBuffer()
{
	return &vertexBuffer;
}

void Mesh::destroyVertexBuffer()
{
	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
}

void Mesh::destroyIndexBuffer()
{
	vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
	vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
}

/*
uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	//Get properties of physical device Memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);		//use memoryProperties to check which of them are compatible with VkMemoryRequirements


	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ( (allowedTypes & (1 << i))														//Index of memory type must match coresponding bit in allowed type       1 shifted left i times s
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)	//Desired property flags are part of memory types property types
		{
			//This memory type is valid so return its index
			return i;
		}
	}
}
	*/

/*
void Mesh::createVertexBuffer(std::vector<Vertex>* vertices)
{
	//CREATE VERTEX BUFFER
	VkBufferCreateInfo createBufferInfo = {};
	createBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//createBufferInfo.queueFamilyIndexCount = 1; //graphics family
	createBufferInfo.size = sizeof(Vertex)*vertices->size() + sizeof(Vertex)*vertices->size()/5;
	createBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	createBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  //shoulde be shared or not perhaps concurent for parallel stuff
															   //createBufferInfo.pQueueFamilyIndices

	VkResult result = vkCreateBuffer(logicalDevice, &createBufferInfo, nullptr, &vertexBuffer);//TODO:destroy
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vertex Buffer");
	}

	//GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, vertexBuffer, &memoryRequirements);

	//ALOCATE MEMORY TO BUFFER
	int sizeCheck = memoryRequirements.size;;
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	//Index of memory type that has required bit flags
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT			//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT memory visible for the cpu -> suboptimal
	//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT after map put directly into the buffer(otherwise will have to specify manually)

	//Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, &vertexBufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Allocate Vertex Buffer Memory");
	}

	//Allocate memory to given vertex buffer
	vkBindBufferMemory(logicalDevice, vertexBuffer, vertexBufferMemory, 0);


	//MAP MEMORY TO VERTEX BUFFER
	void* data;																	//1. create pointer to a point in normal memory
	vkMapMemory(logicalDevice, vertexBufferMemory, 0, createBufferInfo.size, 0, &data);	//Map the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)createBufferInfo.size);						//Copy from vertices vector to the data point
	vkUnmapMemory(logicalDevice, vertexBufferMemory);							//if data not coherent we should use flushing
}
	
	*/

/*
void Mesh::createBufferAndAlloc(VkPhysicalDevice newPhysicalDevice, VkDevice newLogicalDevice, VkBufferUsageFlags bufferType, VkDeviceSize bufferSize, VkBuffer * buffer, VkMemoryPropertyFlags memoryProperties, VkDeviceMemory * bufferMemory)
{
	//CREATE VERTEX BUFFER
	VkBufferCreateInfo createBufferInfo = {};
	createBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//createBufferInfo.queueFamilyIndexCount = 1; //graphics family
	createBufferInfo.size = bufferSize;
	createBufferInfo.usage = bufferType;//VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	createBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  //shoulde be shared or not perhaps concurent for parallel stuff
															   //createBufferInfo.pQueueFamilyIndices

	VkResult result = vkCreateBuffer(newLogicalDevice, &createBufferInfo, nullptr, buffer);//TODO:destroy
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vertex Buffer");
	}

	//GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(newLogicalDevice, *buffer, &memoryRequirements);

	//ALOCATE MEMORY TO BUFFER
	int sizeCheck = memoryRequirements.size;;
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
											memoryProperties); //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

															   //Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(newLogicalDevice, &memoryAllocateInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Allocate Vertex Buffer Memory");
	}

	//Allocate memory to given vertex buffer
	vkBindBufferMemory(newLogicalDevice, *buffer, *bufferMemory, 0);
} 
*/

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
{
	VkDeviceSize bufferSize = sizeof(Vertex)* vertices->size();
	//TODO: Wrap in one func and move to utilities
	/*
	//Create Vertex Buffer
	createBufferAndAlloc(physicalDevice, logicalDevice,
	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize, &vertexBuffer,
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBufferMemory);

	//MAP MEMORY TO VERTEX BUFFER
	void* data;																	//1. create pointer to a point in normal memory
	vkMapMemory(logicalDevice, vertexBufferMemory, 0, bufferSize, 0, &data);	//Map the vertex buffer memory to that point
	memcpy(data, vertices->data(), (size_t)bufferSize);						//Copy from vertices vector to the data point
	vkUnmapMemory(logicalDevice, vertexBufferMemory);							//if data not coherent we should use flushing

	*/

	for(int i = 0; i < vertices->size(); i++)
	{
		glm::vec3 tmpNormal =(vertices->at(i)).normal;
		//Vertex tmpVertex = *vertices->data*i;
		
	}


	//Create Staging Buffer
	createBufferAndAlloc(physicalDevice, logicalDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, &stagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);

	//MAP MEMORY TO STAGING BUFFER -> buffer to prepare memory for thransfer
	void* stagingData;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &stagingData);
	memcpy(stagingData, vertices->data(), bufferSize);
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	//Create buffer with transfer DST bit and Vertex bit -> both transfer and vertex buffer
	//MEMORY VISIBLE ONLY BY GPU -> VkMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	createBufferAndAlloc(physicalDevice, logicalDevice,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bufferSize, &vertexBuffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBufferMemory);

	copyBuffer(logicalDevice, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize); //transfer command buffer is inside

	//Clean Staging buffer 
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);

}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size() ;
	//TODO: use same staging buffer with offset
	//temp staging buffer
	VkBuffer stagingIndBuffer;
	VkDeviceMemory stagingIndBufferMemory;

	//This is transfer src buffer
	createBufferAndAlloc(physicalDevice, logicalDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, &stagingIndBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingIndBufferMemory);

	//MAP MEMORY TO INDEX BUFFER
	void* data;
	vkMapMemory(logicalDevice, stagingIndBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices->data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDevice, stagingIndBufferMemory);

	// Create buffer of index data on GPU
	createBufferAndAlloc(physicalDevice, logicalDevice,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bufferSize, &indexBuffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBufferMemory);

	copyBuffer(logicalDevice, transferQueue, transferCommandPool,
		stagingIndBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(logicalDevice, stagingIndBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingIndBufferMemory, nullptr);
	
}

int Mesh::getIndcesCount()
{
	return indexCount;
}

VkBuffer Mesh::getIndexBuffer()
{
	return indexBuffer;
}

void Mesh::calcNormal(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	//TODO: Make one big chunk of memory for vertices, colors, textures and normals instead different containers  
	//TODO CHECK CLOCKWISE, COUNTERCLOCKWISE
	uint32_t vertInd0, vertInd1, vertInd2;
	for (uint32_t i = 0; i < indexCount; i +=3 ) 
	{
		vertInd0 = indices[i];
		vertInd1 = indices[i + 1];
		vertInd2 = indices[i + 2];

		glm::vec3 edge01 = vertices[vertInd1].pos - vertices[vertInd0].pos;
		glm::vec3 edge02 = vertices[vertInd2].pos - vertices[vertInd0].pos;

		vertices[vertInd0].normal += glm::normalize(glm::cross(edge02, edge01));
		vertices[vertInd1].normal += glm::normalize(glm::cross(edge02, edge01));
		vertices[vertInd2].normal += glm::normalize(glm::cross(edge02, edge01));
	}

	for(uint32_t i = 0; i < verticesCount; i++)
	{
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
}

int Mesh::getTexID()
{
	return texId;
}




 