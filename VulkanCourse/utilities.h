#pragma once
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/glm.hpp>
const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//Indices of the Queue Families (if they exist at all)
struct QueueFamilyIndicies
{
	int graphicsFamily = -1;     //location of graphics queue family
	int presentationFamily = -1; //location of presentation family

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};
 

struct Vertex
{
	glm::vec3 pos; //vertex pos x,y,z
	glm::vec3 col; //vertex Color r,g,b
	glm::vec2 tex; //vertex texel u,v -> wil use r32g32 format
	glm::vec3 normal;
};

struct SwapChainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		 //surface propertirs e.g sirface size/extent
	std::vector<VkSurfaceFormatKHR> listOfSurfaceFormat; //e.g. rgba and size of each channel
	std::vector<VkPresentModeKHR>   listOfPresentMode;   //how images shoild be presented to screen eg. mailbox, fifo
};
struct SwapChainImage {
	VkImage image; 
	VkImageView imageView;
};

struct modelMaterials
{
	int modelID = 0; //TODO: untie from tis struct
	std::string materialName;
	std::vector<std::string> modelTexturesFileNames;
	std::vector<int> modelTexturesIDs;
	std::vector<glm::vec3> colorMaterials;
	int availableTexturesCount = 0;
};

static std::vector<char>readFile(const std::string &filename)
{
	//open stream from given file
	//std::ios::binary tells to read file as binary
	//std::ios::ate put pointer at end ->(ate)
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	//check if filestream succesfuly opened
	if(!file.is_open())
	{
		throw std::runtime_error("Failed to open a file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);
	
	//move read position (seek to) the start of the file
	file.seekg(0);
	file.read(fileBuffer.data(), fileSize);
	//close stream
	file.close();
	return fileBuffer;
};

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	//Get properties of physical device Memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);		//use memoryProperties to check which of them are compatible with VkMemoryRequirements


	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))														//Index of memory type must match coresponding bit in allowed type       1 shifted left i times s
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)	//Desired property flags are part of memory types property types
		{
			//This memory type is valid so return its index
			return i;
		}
	}
}

static void createBufferAndAlloc(VkPhysicalDevice newPhysicalDevice, VkDevice newLogicalDevice, VkBufferUsageFlags bufferType, VkDeviceSize bufferSize, VkBuffer * buffer, VkMemoryPropertyFlags memoryProperties, VkDeviceMemory * bufferMemory)
{
	//CREATE  BUFFER
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
		throw std::runtime_error("Failed to create  Buffer");
	}

	//GET BUFFER MEMORY REQUIREMENTS
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(newLogicalDevice, *buffer, &memoryRequirements);

	//ALOCATE MEMORY TO BUFFER
	VkDeviceSize sizeCheck = memoryRequirements.size;;
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(newPhysicalDevice, memoryRequirements.memoryTypeBits,
		memoryProperties); //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

						   //Allocate memory to VkDeviceMemory
	result = vkAllocateMemory(newLogicalDevice, &memoryAllocateInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Allocate Buffer Memory");
	}

	//Allocate memory to given vertex buffer
	vkBindBufferMemory(newLogicalDevice, *buffer, *bufferMemory, 0);
}

static VkCommandBuffer createAndBeginTmpCommandBuffer(VkDevice newDevice,VkQueue queue, 
										VkCommandPool commandPool)
{
	//Temp Command buffer to hold  commands
	VkCommandBuffer tmpCommandBuffer;

	VkCommandBufferAllocateInfo cmdBuffInfo = {};
	cmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffInfo.commandPool = commandPool;
	cmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffInfo.commandBufferCount = 1;

	//Allocate buffer command buffer memory info
	vkAllocateCommandBuffers(newDevice, &cmdBuffInfo, &tmpCommandBuffer);

	//Information to begin command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //Use it one time in this case
		
	vkBeginCommandBuffer(tmpCommandBuffer, &beginInfo);

	return tmpCommandBuffer;
}

static void EndAndSubmitTmpCommandBuffer(VkDevice newDevice,VkCommandBuffer* tmpCommandBuffer, 
								 VkQueue queue, VkCommandPool commandPool)
{
	vkEndCommandBuffer(*tmpCommandBuffer);

	//Queue submission information //no binding??
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = tmpCommandBuffer ;
	//for large scene -> some sync and stuff
	//Submit transfer to transferQueue(graphics queue) and wait untill it finishes
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	//Free temporary command buffer back to pool
	vkFreeCommandBuffers(newDevice, commandPool, 1, tmpCommandBuffer);
	//do not destroy, bacause grapficsCommandPool is renderer is used as transferCommandPool and it is destroyed in renderer
}

static void copyBuffer(VkDevice newDevice,
	VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	/*
	//TODO: Add dependency injection
	//Temp Command buffer to hold transfer commands TODO: make static in future
	VkCommandBuffer transferCommandBuffer;

	//Allocate buffer command buffer memory info
	VkCommandBufferAllocateInfo transCmdBuffInfo = {};
	transCmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	transCmdBuffInfo.commandPool = transferCommandPool;
	transCmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	transCmdBuffInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(newDevice, &transCmdBuffInfo, &transferCommandBuffer);

	//Information to begin command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //Use it one time in this case

	*/

	VkCommandBuffer transferCommandBuffer = createAndBeginTmpCommandBuffer(newDevice, transferQueue, transferCommandPool);
														
	// Region of data to copy from and to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	//Command to copy sorce buffer to destination buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);//we can copy array of regions
																					   
	/*
	//Queue submission information //no binding??
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;
	//for large scene -> some sync and stuff
	//Submit transfer to transferQueue(graphics queue) and wait untill it finishes
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);
	*/
	
	EndAndSubmitTmpCommandBuffer(newDevice, &transferCommandBuffer, transferQueue, transferCommandPool);
	//do not destroy, bacause grapficsCommandPool is renderer is used as transferCommandPool and it is destroyed in renderer
}

static void copyBufferToImage(VkDevice newDevice,VkQueue transferQueue, VkCommandPool transferCommandPool, 
			VkBuffer srcBuffer,VkImage dstImage,
			int width, int height,	VkDeviceSize bufferSize)
{


	VkCommandBuffer transferCommandBuffer = createAndBeginTmpCommandBuffer(newDevice, transferQueue, transferCommandPool);
	
	VkBufferImageCopy imageRegion = {};
	imageRegion.bufferOffset = 0;
	imageRegion.bufferRowLength = 0;
	imageRegion.bufferImageHeight = 0;
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageRegion.imageSubresource.mipLevel = 0;
	imageRegion.imageSubresource.baseArrayLayer = 0;
	imageRegion.imageSubresource.layerCount = 1;
	imageRegion.imageOffset = { 0, 0, 0 };			//xyz
	imageRegion.imageExtent.width = (uint32_t) width;
	imageRegion.imageExtent.height = (uint32_t)height;
	imageRegion.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImage, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

EndAndSubmitTmpCommandBuffer(newDevice, &transferCommandBuffer, transferQueue, transferCommandPool);

};

