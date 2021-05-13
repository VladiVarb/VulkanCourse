#pragma once

//#define VK_USE_PLATFORM_WIN32_KHR


#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>


#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include "utilities.h"
#include <array>
#include "Mesh.h"
#include "MeshModel.h"
#include "Camera.h"

//#include "stb_image.h"

const int MAX_FRAMES_DRAWS = 2;
const size_t MAX_OBJECTS = 20;
class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow * newWindow);
	void  draw();
	~VulkanRenderer();
	void cleanup();
	void updateModel(int modelID, glm::mat4 newModel);
	GLFWwindow* window;
	Camera camera;
	/*
	struct PV {
		glm::mat4 projection;
		glm::mat4 view;
	}	pv;
	*/
	MeshModel* getModel(int modelID);
private:
	
	 
	 int currentFrame = 0;

	 //Scene Objects
	 //std::vector<Mesh> meshList ;
	// Mesh firstMesh;
	// Mesh secondMesh;

	 //Scene setting 
	
	 // Vulkan components
	 // - Main 
	 VkInstance instance;
	 //add debugger here
	 struct {
		 VkPhysicalDevice physicalDevice;
		 VkDevice logicalDevice;
	 } mainDevice;

	 VkQueue graphicsQueue;
	 VkQueue presentationQueue;
	 VkSurfaceKHR surface;
	 VkSwapchainKHR swapChain;
	 
	 std::vector<SwapChainImage> swapChainImages; //struct from the utilities.h
	 std::vector<VkFramebuffer> swapChainFramebuffers;
	 std::vector<VkCommandBuffer> commandBuffers;


	 //vulkan functions 
	 //  - Create functions
	 void createInstance();
	 void createLogicalDevice();
	 void createSurface();
	 void createSpawChain();

	 void createColorBufferImage();
	 void createDepthBufferImage(/*will see*/); // will create and update the depthImage and depthImageView, NOTHING MORE.
	

	 //Descriptors ->uniform, texture, color, depth
	 void creatDescriptorSetLayouts();				//uniform, texture, color, depth
	
	 //Uniform Descriptors								  
	 void createUniDescriptorSetBuffers();
	 void createUniDescriptorsPool();
	 void createUniDescriptorSets();				//sets and buffer are get connected here
	
				
	 
	 //TODO: try to create descLayout on vertex stage
	 void createGraphicsPipeline();
	 void createRenderPass();
	 void createFramebuffers();
	 void createColorDepthDescriptorPool();
	 void createColorDepthDescSets();

	 //Textures
	 void mapTexNamesToTexIDs(std::vector<std::string> &texNamesPool, modelMaterials& modelTex); //maps texture names to createTextures ID
	 void updateIDs(std::string str, int pos, std::vector<std::string>& pool, modelMaterials& modelTex);
	 void loadTexName(std::string fileName);
	 void createTexFromTexPool();		//wrap function 
	 void createTextureImage(int pos);						//Load raw image file and transfer it to VulkanTexture file
	 VkSampler createTextureSampler();						//used in createTextureDesc(textureImageView)
	 void createSamplerDescriptorPool();
	 int createTextureDesc(VkImageView textureImageView);
	 //std::vector<aiString> meshNames;
	 void createCommandPool();
	 void createCommandBuffers();
	 void createSynchronisation();
	 
	 //Validation layers
	 VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
	 void initDebug();
	 std::vector<const char*> layerNamesList;
	 PFN_vkCreateDebugReportCallbackEXT pfvkCreateDebugReportCallbackEXT = nullptr;
	 PFN_vkDestroyDebugReportCallbackEXT pfvkDestroyDebugReportCallbackEXT = nullptr;
	 VkDebugReportCallbackEXT pDebugReport = nullptr;
	 static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(VkDebugReportFlagsEXT msgFlags,
														VkDebugReportObjectTypeEXT objType,
														uint64_t srcObj,
												size_t location,
														int32_t msgCount,
														const char* layerPrefix,
														const char* message,
														void* userData );
	

	 void stopDebug();
	 // - Assets 
	 std::vector<VkImage> textureImages;
	 std::vector<VkImageView> textureImageViews;
	 std::vector<VkDeviceMemory> textureImagesMemory;
	 std::vector<MeshModel> meshModels;
	 std::vector<std::string> textureNamesPool;
	
	 //Textures
	 // stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);
	 VkDeviceSize loadedTextureSize;	//will be used for first staging buffer size
	 VkBuffer rawTexStagingBuffer;
	 VkDeviceMemory rawTexStagingBufferMemory;
	 std::vector<modelMaterials> modelsMaterialsList;
	 int numTextures = 0;
	 VkDescriptorSetLayout samplerDescLayout;
	 VkSampler sampler;
	 VkDescriptorPool TexturesDescPool;
	 std::vector<VkDescriptorSet> TexturesDescSets;  //Not going to change, therefore NO Need to create one for each swapImage, just the size of textures vec
	
	 // Descriptors subpass
	 //color and depth
	// use the color and depth images instead buffer //std::vector<VkBuffer> colorDepthBuffer;
	 VkDescriptorSetLayout colorDepthDescLayout;
	 VkDescriptorPool colorDepthDescPool;
	 std::vector<VkDescriptorSet> colorDepthDescSets;
	 // - Descriptor Buffers & Memory & Pools & Sets

	 VkPushConstantRange pushConstantRange;
	 void initPushConstRange();

	 std::vector<VkBuffer> descriptorBuffersSta;
	 std::vector<VkDeviceMemory>descriptorsMemorySta;
	 VkDescriptorPool descriptorPool;
	 std::vector<VkDescriptorSet> descriptorSets;
	 void updateUniDescSets(uint32_t frameIndex); // will change desc memories vkMapMem //will be used in draw() function
	//Dynamic Descriptors
	 
	 void getMinimalAlignmentAvailable(VkPhysicalDevice physicalDevice);
	 uint32_t minAligmentAvailable; //FOR model
	 
	void allocDynBuffTransferSpace();
	VkDeviceSize alignedSpacePerModelSize;//model unifor alignemt
	UboModelStruct* ptrDynBuffTransferSpace ; //TODO try with void in the end
	 
	 //std::vector<VkBuffer> descriptorBuffersDyn;
	 //std::vector<VkDeviceMemory>descriptorsMemoryDyn;
	 							   
	 // -- DescriptorS Set Layout
	 VkDescriptorSetLayout descriptorSetLayout;


	 //Color for the color attachment in the subpasses
	 std::vector<VkImage> colorImages;
	 std::vector<VkImageView> colorImageView;
	 std::vector<VkDeviceMemory> colorMemory;
	 // Depth 
	 //not vector, because framebuffer will use(check and update one depth image/image view)
	 //up is not a case anymore because we will be using two depths for a frame; 
	 std::vector<VkImage> depthImages;
	 std::vector <VkImageView> depthImageView;
	 std::vector<VkDeviceMemory> depthMemory;


	 std::vector<VkFormat> formats{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };
	 // - Pipeline
	 VkPipeline graphicsPipeline;
	 VkPipelineLayout pipelineLayout;  //will pass descriptors to it
	 
	 VkPipeline secondPipeline;
	 VkPipelineLayout secondPipeLayout;


	 VkRenderPass renderPass;

	 // - Pools - 
	 VkCommandPool graphicsCommandPool;


	 // - Utility
	 VkFormat swapChainImageFormat;
	 VkExtent2D swapChainExtent; //resoilution and stuff
	 
 
	 // - Synchronisation -
	 std::vector<VkSemaphore> imageAvailableSemaphores;
	 std::vector<VkSemaphore> renderFinishedSemaphores;
	 std::vector<VkFence> drawFences;

	 // - Get Functions
	 void getPhysicalDevice();

	 // - Record Function -
	 void recordCommands(uint32_t currentFrame);

	 // - Support Functions
	 void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	
	
	 // -- Checker functions
	 bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensons);
	 bool checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice);
	// bool checkDeviceSurfaceCapabilities(VkPhysicalDevice physicalDevice); //trim maybe
	 bool checkDeviceSuitable(VkPhysicalDevice physicalDevice);
	 
	 // -- Getter families function 
	 QueueFamilyIndicies getQueFamilyIndices(VkPhysicalDevice physicalDevice);
	 SwapChainDetails getSwapChainDatails(VkPhysicalDevice physicalDevice);
	 
	 // -- Chooser functions
	 VkFormat chooseSupportedImageFormat(std::vector<VkFormat> &formats,VkImageTiling tiling, VkFormatFeatureFlags features);
	 VkSurfaceFormatKHR chooseBestSurfaceFormat(std::vector<VkSurfaceFormatKHR> &formats);
	 VkPresentModeKHR chooseBestPresentMode(std::vector<VkPresentModeKHR> &presentatioMode);
	 VkExtent2D chooseSwapExtend(const VkSurfaceCapabilitiesKHR &surfaceCapabilities); //resolution

	 // -- Create functions (support functions)
	 void createMeshModel(std::string fileName, std::vector<MeshModel> &newMeshModelList);
	 VkImage createImage(VkFormat format, VkExtent2D extent2d, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkDeviceMemory* depthMem, VkMemoryPropertyFlags memoryProperties);
	 VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	 VkShaderModule createShaderModule(const std::vector<char> &code);
};

