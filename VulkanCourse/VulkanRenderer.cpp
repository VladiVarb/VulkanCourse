
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "VulkanRenderer.h"

#define NOMINMAX
#ifdef _WIN32
	#include <Windows.h>
	#include <sstream>
#endif

stbi_uc* loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);


VulkanRenderer::VulkanRenderer()
{

}

int VulkanRenderer::init(GLFWwindow * newWindow)
{
	window = newWindow;
	
	try {
		//Mesh::destroyVertexBuffer(mainDevice.logicalDevice, )
		createInstance();
		initDebug();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();		
		createSpawChain();

		
		createRenderPass();

		initPushConstRange();
		creatDescriptorSetLayouts();
		sampler = createTextureSampler();

		createGraphicsPipeline();

		createColorBufferImage();
		createDepthBufferImage();

		createFramebuffers();
		createCommandPool();
		//Add Vert and Create Mesh
		camera.pv.projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);

		//pv.projection = glm::perspective(glm::radians(45.0f), (float)swapChainExtent.width/(float)swapChainExtent.height, 0.1f, 500.0f );
		
		//lookAt  (eye ->where the camera is positioned, center-> , up-> which ax is up*(inverse from glm /opengl standart))
		camera.pv.view = glm::lookAt(glm::vec3(200.0f, 0.f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		camera.pv.projection[1][1] *= -1;
		//camera.pv.lightDirection = glm::vec3(1.f, -1.0f, 2.0f);

		//TODO: use offset for vertex buffers
		createCommandBuffers();
		//Descriptors

		//Dynamic Descriptors
		
		//comment because we do not use them -. push constants instead
		//getMinimalAlignmentAvailable(mainDevice.physicalDevice);
		//allocDynBuffTransferSpace();

		createUniDescriptorsPool();
		createSamplerDescriptorPool();
		
		createUniDescriptorSetBuffers();
		

		createUniDescriptorSets();
		//Descriptors End here

		//record commands was here before using push constants 
		//now it is in draw()
		createColorDepthDescriptorPool();
		createColorDepthDescSets();
		createSynchronisation();



		//Vertex data
		/*
		std::vector<Vertex> firstMeshVertices = {
			{ { 0.4, -0.4, 0.0 },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } }, // 0    Vertex = {{pos}, {col}, {tex}}
			{ { 0.4,  0.4, 0.0 },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } }, // 1
			{ { -0.4, 0.4, 0.0 },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } }, // 2
			{ { -0.4, -0.4, 0.0 },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },{}   // 3		
		};

		std::vector<Vertex> secondMeshVertices = {
			{ { 0.45, -0.2, 0.0 },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } }, // 0    Vertex = {{pos}, {col}}
			{ { 0.45,  0.2, 0.0 },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } }, // 1
			{ { -0.45, 0.2, 0.0 },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } }, // 2
			{ { -0.45, -0.2,0.0 },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },{}  // 3		
		};

		//Index data
		std::vector<uint32_t> meshIndices = {
			0, 1, 2,
			2, 3, 0
		};



		//TODO: transformation function 
		firstMesh = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice,
			graphicsQueue, graphicsCommandPool,
			&firstMeshVertices, &meshIndices, createTexture("red_paint_tex.jpg"));
		
		secondMesh = Mesh(mainDevice.physicalDevice, mainDevice.logicalDevice,
			graphicsQueue, graphicsCommandPool,
			&secondMeshVertices, &meshIndices, createTexture("red_paint_tex.jpg"));

		meshList.push_back(firstMesh);
		meshList.push_back(secondMesh);
		*/
		//FOR test puroses
		/*
		glm::mat4 newModel = glm::rotate(meshList[0].getUboModelStr().model,
		glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		meshList[0].setModel(newModel);
		*/
		createMeshModel("Models/Seahawk.obj", meshModels);
		createTexFromTexPool();
	}
	catch (const std::runtime_error &e)
	{
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}
	return 0;
}

 void VulkanRenderer::draw()
{ 
	// - GET INAGE-   -> next available image to draw and set something to signal when finnished with image (a semaphore)
	uint32_t imageIndex; //the index in swapchain
	vkAcquireNextImageKHR(mainDevice.logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);//drawFences[currentFrame]  
	

	vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()); //stop until fence is opened. It will be opened  																														//manualy reset (close) fence for this frame
	
	updateUniDescSets(imageIndex); 
	recordCommands(imageIndex);																																								  // wait	for given fence to signal (open) from last drawbefor continiuing																																							  //
	
	vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]); //close the fence
																																
																															//will mess sequence ??!!
	// - SUBMIT COMMAND BUFFER TO RENDER
	//	Queue submission
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	VkPipelineStageFlags stageToWaitFor[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = stageToWaitFor;
	submitInfo.commandBufferCount = 1;						  // We want semaphore to work on current buffer only
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; //
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Submit to Command Buffer to Queue");
	}
	// - PRESENT iMAGE - 

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;
	
	result = vkQueuePresentKHR(presentationQueue, &presentInfo);

	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Present Image to Queue");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_DRAWS;
}


VulkanRenderer::~VulkanRenderer()
{

}

void VulkanRenderer::cleanup()
{


	//Wait untill no action has been run on device before destroying 
	vkDeviceWaitIdle(mainDevice.logicalDevice); //or vkQueueWaitIdle()
	
	
	for (auto &model : meshModels)
	{
		model.destroyMeshModel();
	}

	vkDestroySampler(mainDevice.logicalDevice, sampler, nullptr);
	vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, samplerDescLayout, nullptr);
	vkDestroyDescriptorPool(mainDevice.logicalDevice, TexturesDescPool, nullptr);
	
	for(size_t i = 0; i < textureImageViews.size(); i++)
	{
		vkDestroyImageView(mainDevice.logicalDevice, textureImageViews[i], nullptr);
	}


	for (size_t i = 0; i < textureImages.size(); i++)
	{
		vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, textureImagesMemory[i], nullptr);
	}

	// _aligned_free(ptrDynBuffTransferSpace);
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyImageView(mainDevice.logicalDevice, depthImageView[i], nullptr);
		vkDestroyImage(mainDevice.logicalDevice, depthImages[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, depthMemory[i], nullptr);
	}

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyImageView(mainDevice.logicalDevice, colorImageView[i], nullptr);
		vkDestroyImage(mainDevice.logicalDevice, colorImages[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, colorMemory[i], nullptr);
	}


	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyBuffer(mainDevice.logicalDevice, descriptorBuffersSta[i], nullptr);
		vkFreeMemory(mainDevice.logicalDevice, descriptorsMemorySta[i], nullptr);

		//vkDestroyBuffer(mainDevice.logicalDevice, descriptorBuffersDyn[i], nullptr);
		//vkFreeMemory(mainDevice.logicalDevice, descriptorsMemoryDyn[i], nullptr);
	}

	vkDestroyDescriptorPool(mainDevice.logicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorPool(mainDevice.logicalDevice, colorDepthDescPool, nullptr);
	vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, colorDepthDescLayout, nullptr);
    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, descriptorSetLayout, nullptr);
	/*
	for (auto& mesh : meshList) 
	{
		mesh.destroyIndexBuffer();
		mesh.destroyVertexBuffer();
	}
	*/

	for (size_t i = 0; i < MAX_FRAMES_DRAWS; i++) 
	{
		vkDestroySemaphore(mainDevice.logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(mainDevice.logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
	}
	
	vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);
	for (auto framebuffer : swapChainFramebuffers) 
	{
		vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
	}

	vkDestroyPipeline(mainDevice.logicalDevice, secondPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice.logicalDevice, secondPipeLayout, nullptr);

	vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(mainDevice.logicalDevice, renderPass, nullptr);

	

	for(auto image : swapChainImages)
	{
		vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr); 
	}
	vkDestroySwapchainKHR(mainDevice.logicalDevice, swapChain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);

	stopDebug();
	vkDestroyDevice(mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr);

	
}

void VulkanRenderer::createInstance()
{
	//info about application itself
	//most data here is for convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "vULKAN APP"; //custom name of the app
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 148);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	//this will affect program
	appInfo.apiVersion = VK_API_VERSION_1_1;//TRY WITH REAL 1_2
	//Debug

	

	//create info for vulkan instance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	//createInfo.pNext -> used for extesions of the struct
	createInfo.pApplicationInfo = &appInfo;
	
	//create list to hold instance extensions(this are required xtensions)
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	//set up extensions instance will use
	uint32_t glfwExtensionsCount = 0; //GLFW may require multiple extensions
	const char** glfwExtensions;     //extensions passed as arr cstring
	
	//get glfw extensions 
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	//Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionsCount; i++)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("VkInstance does not support reqiured extensions");
	}

	//Validation Layers
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> layersProperitesList(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, layersProperitesList.data());

	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//for (auto &layer : layersProperitesList)
	//{
		layerNamesList.push_back("VK_LAYER_KHRONOS_validation");
	//}
	//
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data(); 

	//TODO: Set up Validation Layers that Instance will use
	createInfo.enabledLayerCount = static_cast<uint32_t>(layerNamesList.size());
	createInfo.ppEnabledLayerNames = layerNamesList.data(); //use in device creation
	//createInfo.pNext = &debugCreateInfo;
	//Create instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if(result!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan instance");
	}
}

void VulkanRenderer::createLogicalDevice()
{
	//get queue famili indice for the create info
	QueueFamilyIndicies indicies = getQueFamilyIndices(mainDevice.physicalDevice); //this one runs

	//vector for queue creation information and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndicies = { indicies.graphicsFamily,indicies.presentationFamily };


	//QUEUE THE LOGICAL DEVICE NEEDS TO CREATE  and info to do so(only 1 for now)
	float priority = 0.0f;
	for (int queueFamilyIndex : queueFamilyIndicies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = (uint32_t)queueFamilyIndex; //(uint32_t)indicies.graphicsFamily;
		queueCreateInfo.queueCount = (uint32_t)queueFamilyIndicies.size(); // for now
		priority += (float)(1.f / (float)queueFamilyIndicies.size());
		//++priority;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}
	//vulkan needs to know how to handle multiple queues

   //information to create logical device
	VkDeviceCreateInfo deviceCreateInfo = {};
	std::vector<VkDeviceCreateInfo> deviceCreateList(1);
	for (auto & deviceInfo : deviceCreateList)
	{
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount =static_cast<uint32_t>(deviceExtensions.size());		//number of enabled logical device extensions
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();						    //list of enabled logical device extensions
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layerNamesList.size());
		deviceCreateInfo.ppEnabledLayerNames = layerNamesList.data();
																									//Physical device feature the logical device will be using
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		
	}
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Could not create a logical device");
	}

	//Queues are created at same time as the device,
	//so we want handle to queues 
	//from given logical device, of Queue Family of givenQueindex0, since only one queue) 

	vkGetDeviceQueue(mainDevice.logicalDevice, (uint32_t)indicies.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, (uint32_t)indicies.presentationFamily, 0, &presentationQueue);
}

void VulkanRenderer::createSurface()
{
	//create surface (info struct runs the create surface func)
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (result!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create surface");
	}

}

void VulkanRenderer::createSpawChain()
{
	//get swapcahin details sop we can pick best settings
	SwapChainDetails swapChainDetails = getSwapChainDatails(mainDevice.physicalDevice);					//CreationStruct
	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.listOfSurfaceFormat);	//Choose Best Surface format == ImageFormat
	VkPresentModeKHR presentMode = chooseBestPresentMode(swapChainDetails.listOfPresentMode);			//Chose best presentation mode
	VkExtent2D resolution = chooseSwapExtend(swapChainDetails.surfaceCapabilities);						//Choose swap best resolution

	//how many images are in the swap chain? Get one morew than minimum to allow tripple buffering
	uint32_t  imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;
	if(imageCount > swapChainDetails.surfaceCapabilities.maxImageCount
		&& swapChainDetails.surfaceCapabilities.maxImageCount == 0)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount ;
	}

	//creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//swapChainInfo.flags = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR; //by me
	swapChainInfo.surface = surface; //by me
	swapChainInfo.imageFormat = surfaceFormat.format;
	swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainInfo.imageExtent = resolution; 
	swapChainInfo.presentMode = presentMode;
	swapChainInfo.minImageCount = imageCount;  //images in swapchain
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //what attachment images will be used at
	swapChainInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform; //transform to perform chainswap images
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // how to blend images wiht external graphics()windows
	swapChainInfo.clipped = VK_TRUE; //clip part of window behind another image(e.g behind another window, off screen)

	//get queue family indices
	QueueFamilyIndicies indices = getQueFamilyIndices(mainDevice.physicalDevice);

	//if graphics and presentation families are different,
	//then swapchain must let images to sharedbetween families
 const uint32_t ind{ (uint32_t)indices.graphicsFamily };

uint32_t queueFamilyindices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

	if (indices.graphicsFamily != indices.presentationFamily)
	{
		
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainInfo.queueFamilyIndexCount = 2;                //use graphics familly and presentation failly
		swapChainInfo.pQueueFamilyIndices = queueFamilyindices; //array of queues to share between
	}
	else
	{
		
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainInfo.queueFamilyIndexCount = 1;//0
		swapChainInfo.pQueueFamilyIndices = queueFamilyindices; //nullptr
	}

	//if old swapchain been destroyed and this replaces it, then link the old one to quickly
	//hand over responsibilities
	swapChainInfo.oldSwapchain = VK_NULL_HANDLE;

	//create swapchain
	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainInfo, nullptr, &swapChain);
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swapchain");
	}

	//Store for later refference
	swapChainImageFormat = surfaceFormat.format; //save to var in .h
	swapChainExtent = resolution;  
	//get swapchain images first count then values
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount); //images in swapchain
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, images.data());
	
	for(VkImage image : images )
	{
		//store handle
		SwapChainImage tempSwapChainImage = {};
		tempSwapChainImage.image = image;
		tempSwapChainImage.imageView = createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		swapChainImages.push_back(tempSwapChainImage); 
	}

}

void VulkanRenderer::createColorBufferImage()
{
	colorImages.resize(swapChainImages.size());
	colorImageView.resize(swapChainImages.size());
	colorMemory.resize(swapChainImages.size());
	/*
	std::vector<VkFormat> format = { VK_FORMAT_R8G8B8A8_UNORM };
	VkFormat colorFormat = chooseSupportedImageFormat(
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	*/
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		colorImages[i] = createImage(swapChainImageFormat,// colorFormat
			swapChainExtent,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			&colorMemory[i],
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		colorImageView[i] = createImageView(colorImages[i],
			swapChainImageFormat,// colorFormat
			VK_IMAGE_ASPECT_COLOR_BIT);
	}
}
void VulkanRenderer::createDepthBufferImage()
{
	depthImages.resize(swapChainImages.size());
	depthImageView.resize(swapChainImages.size());
	depthMemory.resize(swapChainImages.size());

	VkFormat depthFormat = chooseSupportedImageFormat(formats,
	VK_IMAGE_TILING_OPTIMAL, {VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT});
	

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		depthImages[i] = createImage(depthFormat,
			swapChainExtent,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			&depthMemory[i], 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		depthImageView[i] = createImageView(depthImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	
}

 //create swapChain ends here

void VulkanRenderer::creatDescriptorSetLayouts()
{
	//Static Descriptors Layout Bindings

	VkDescriptorSetLayoutBinding descLayoutBindingsSta = {};
	descLayoutBindingsSta.binding = 0;											//binding to the set layout
	descLayoutBindingsSta.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descLayoutBindingsSta.descriptorCount = 1;
	descLayoutBindingsSta.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;   //!!VERTEX_BIT because in shader.vert
	descLayoutBindingsSta.pImmutableSamplers = nullptr;              //used for textures
	
	/*
	//Dynamic Descriptors Layout Bindings
	VkDescriptorSetLayoutBinding descLayoutBundingsDyn = {};
	descLayoutBundingsDyn.binding = 1;
	descLayoutBundingsDyn.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descLayoutBundingsDyn.descriptorCount = 1;
	descLayoutBundingsDyn.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descLayoutBundingsDyn.pImmutableSamplers = nullptr;
	*/

	
	std::vector<VkDescriptorSetLayoutBinding> descLayoutsBindings{ descLayoutBindingsSta };

	VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = {};
	descSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descLayoutsBindings.size());
	descSetLayoutCreateInfo.pBindings = descLayoutsBindings.data();

	

	VkResult result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &descSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
	
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Descriptor Layout");
	}

	
	//Texture Descriptor Layout
	VkDescriptorSetLayoutBinding textureLayoutBinding = {};
	textureLayoutBinding.binding = 0;
	textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureLayoutBinding.descriptorCount = 1;
	textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	textureLayoutBinding.pImmutableSamplers =  nullptr;  //NOT &sampler

	std::vector<VkDescriptorSetLayoutBinding> descTexturesLayoutBindings{ textureLayoutBinding };//descLayoutBundingsDyn
	
	VkDescriptorSetLayoutCreateInfo textureDescLayoutCreateInfo = {};
	textureDescLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureDescLayoutCreateInfo.bindingCount = static_cast <uint32_t>(descTexturesLayoutBindings.size());
	textureDescLayoutCreateInfo.pBindings = descTexturesLayoutBindings.data();

	

	result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &textureDescLayoutCreateInfo, nullptr, &samplerDescLayout);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Texture Descriptor Layout");
	}


	VkDescriptorSetLayoutBinding  colorLayoutBinding = {};
	colorLayoutBinding.binding = 0;		//the layout binding in vert frag files
	colorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	colorLayoutBinding.descriptorCount = 1;
	colorLayoutBinding.stageFlags =	VK_SHADER_STAGE_FRAGMENT_BIT;
	colorLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding  depthLayoutBinding = {};
	depthLayoutBinding.binding = 1;		//the layout binding in vert frag files
	depthLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthLayoutBinding.descriptorCount = 1;
	depthLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	depthLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 2> colorDepthBindings{ colorLayoutBinding , depthLayoutBinding };

	VkDescriptorSetLayoutCreateInfo colorDepthLayoutDescCreateInfo = {};
	colorDepthLayoutDescCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	colorDepthLayoutDescCreateInfo.bindingCount = (uint32_t)colorDepthBindings.size();
	colorDepthLayoutDescCreateInfo.pBindings = colorDepthBindings.data();

	result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &colorDepthLayoutDescCreateInfo, nullptr, &colorDepthDescLayout);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create ColorDepth Descriptor Layout ");
	}
}

 
void VulkanRenderer::createGraphicsPipeline()
{
	//Read in SPIR-V code for shaders
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	//Build Shader Modules to link to Graphics Pipeline
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);
	
	//	--SHADER STAGE CREATION INFORMATION--
	// Vertex stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = vertexShaderModule;
	vertexShaderStageCreateInfo.pName = "main";  //entry point into the shader () firs func from vert file

	//Fragment stage creation information
	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = fragmentShaderModule;
	fragmentShaderStageCreateInfo.pName = "main";  //entry point into the shader () first func from frag file

	//Put shader stage create info into array
	//graphics Pipeline creation info requires array of shader stage creates
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	// How Data for single vertex( pos, colour, texture, coords, normals) is stored 
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;										//Can bind multiple streams of data, this defines which one
	bindingDescription.stride = sizeof(Vertex);							//Size of a single vertex
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;			//How to move betweendata after each vertex.(in case of repeating objects/instances)
																		//VK_VERTEX_INPUT_RATE_VERTEX: move on to next vertex(draw all at once little by little)
																		//VK_VERTEX_INPUT_RATE_INSTANCE : draw first instance first and then draws the second instance
	

	//How a data for an attribute is defined within a vertex 
	//vertices and colors are atributes
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;
	
	//Position attribute
	attributeDescriptions[0].binding = 0;							//in vert file there is "invisible " binding = 0 in layout line, should be same as binding in bindingDescription.binding
	attributeDescriptions[0].location = 0;							//location in shader where data will be read from
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;//Format the data will take(also helps describe the size of datastruct)
	attributeDescriptions[0].offset = offsetof(Vertex, pos);		//Where this attribute is defined in vertex

	//Color attribute												
	attributeDescriptions[1].binding = 0;							
	attributeDescriptions[1].location = 1;							
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, col);	

	//Texture attribute
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, tex);

	//Normal
	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex,normal);

	// --VERTEX INPUT  --
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;                               //1 -> one structure
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;				   //List of Vertex Binding Descriptions(data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();	   //List of Vertex Attribute Dexcriptions (data format and where to bind to/from)

	// --INPUT ASSEMBLY --
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;    //Primitive type to assemble 
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;				 //allow  overriding of "strip" topology to start new primitives
	
	//	--VIEWPORT & SCSISSORS --																	 
	
	VkViewport viewPort = {};
	viewPort.x = 0;
	viewPort.y = 0;
	viewPort.width = (float)swapChainExtent.width;
	viewPort.height = (float)swapChainExtent.height;
	viewPort.minDepth = 0.0f;						//min frameBuffer Depth
	viewPort.maxDepth = 0.1f;
	//(viewport->similar to resize an image)


	// Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };					//Offset to use region from
	scissor.extent = swapChainExtent;			//Extent to describe region to use, starting at offset;
	//everything between offset and extent =->visible
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewPort;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;


	//// -- DYNAMIC STATES --
	//std::vector<VkDynamicState> dynamicStateEnables;
	////Dynamic viewport: Can resize in Command Buffer with
	////vkCmdSetViewport(commandbuffer, firstviewport, viewportcount pViewports)
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);  
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);   
	////if do the above destroy the current swapchain and re set all the stuff there and pass it to new swapchain
	//
	////Dynamic states create info
	//VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	//dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicStateCreateInfo.dynamicStateCount = (uint32_t) dynamicStateEnables.size();
	//dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();


	// -- Rasterizer --
	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	
	/*in enable this it will require a gpu fit feature -> Physical Device features -> enable depth clamp = true	*/
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;		 //Change if things beyond near/far are clipped (default) or clapped(flatten to plane)
	
	/*quit with out fragment just get some data and do not draw anytning, get some data and make some storage buffer*/
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE; 
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;	 // How to handle filling points ,USE mODE LINE FOR WIREFRAME for anything othewr than fill -> GPU feature
	rasterizationCreateInfo.lineWidth = 1.0f;					 //anything other than 1 ->gpu extention
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //WINDING TO DETERMINE WHICH IS FRONT

	/*if to use it set debthBiasSlopeFactor, depthBiasClamp, depthBiasConstantFactor */
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;			 //good to stop shadow acne in shadow mapping


	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //Number of samples to use in fragment;

	// -- BLENDING --
	//blending decides how to blend a new color being written to fragment, with old value

	//Blend attachment state (how blending is made)
	VkPipelineColorBlendAttachmentState colorStateAttachment{};
	//To which components to apply blending -> all components
	colorStateAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; 
	colorStateAttachment.blendEnable = VK_TRUE;
	// blending uses (srcColorBlendFactor*newColor)colorBlendsOp(dstColorBlendFactor*oldColor)
	colorStateAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorStateAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; //src is the new alfa
	colorStateAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorStateAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorStateAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorStateAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingCreateInfo.logicOpEnable = VK_FALSE;   //alternative to calculations
	colorBlendingCreateInfo.attachmentCount = 1;
	colorBlendingCreateInfo.pAttachments = &colorStateAttachment;
	
	std::vector<VkDescriptorSetLayout> descLayouts{ descriptorSetLayout ,samplerDescLayout };

	// --PIPELINE LAYOUT (TODO: Apply future descriptor set layout)
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	

	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Pipeline Layout");
	}

	// -- DEPTH STENCIL TESTING --
	// TODO: Check if enough
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; //does depth value exist between two bounds
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;






	// -- GRAPHICS PIPELINE CREATION


	VkGraphicsPipelineCreateInfo  graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2; //vertex and fragment in shaderStages array
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;//no filled with values
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	//graphicsPipelineCreateInfo.pTessellationState
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;  //render pass can be used by multiple types of pipelines
	graphicsPipelineCreateInfo.subpass = 0; //will use only one of the sunpass, create copy of this pipeline to use second

	//Pipeline Derivatives
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1; //this alternative to handle
	result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Graphics Pipeline");

	}

	//BUILD SECOND PIPELINE  AND PIPELINE LAYOUT
	//reuse the codefile  and module.Its copied anyway

	VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
	secondPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	secondPipelineLayoutCreateInfo.setLayoutCount = 1;
	secondPipelineLayoutCreateInfo.pSetLayouts = &colorDepthDescLayout;
	secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(mainDevice.logicalDevice, &secondPipelineLayoutCreateInfo, nullptr, &secondPipeLayout);
	if(result!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Second Graphics Pipeline Layout");

	}
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);


	vertexShaderCode = readFile("Shaders/vert_2.spv");
	fragmentShaderCode = readFile("Shaders/frag_2.spv");

	vertexShaderModule = createShaderModule(vertexShaderCode);
	fragmentShaderModule = createShaderModule(fragmentShaderCode);


	vertexShaderStageCreateInfo.module = vertexShaderModule;
	fragmentShaderStageCreateInfo.module = fragmentShaderModule;

	shaderStages[0] = vertexShaderStageCreateInfo;
	shaderStages[1] = fragmentShaderStageCreateInfo;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.layout = secondPipeLayout;
		//!!!NO VERTEX
	vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;		//List of Vertex Binding Descriptions(data spacing/stride information)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;	//List of Vertex Attribute Dexcriptions (data format and where to bind to/from)

	//NO DEPTH																
	depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;	
	
	//!!!DONT FORGET THE SUBPASS!!! 
	graphicsPipelineCreateInfo.subpass = 1;			//the index of the subass used

	result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &secondPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Second Graphics Pipeline");

	}


	//Destroy shader modules no longer needed keep only if we will reuse them in different pipeline
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
	
}

void VulkanRenderer::createRenderPass()
{

	std::array<VkSubpassDescription, 2> subpasses{};

	// Attachment & Reference for first subpass -> subpasses[0]
	//subpasses[0] -> col_1, depth attachments
	// Attachment -> Color Attachement  Input
	VkAttachmentDescription colorAttachement = {};
	colorAttachement.format = swapChainImageFormat;						//Format to use for attachement
	colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;					//Number Of samples to write for multsisampling
	colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				//What to do with attachement before rendering
	//storeOp do not care -> used ussually for the depth buffer
	colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;			//What to do with attachement after rendering it Store atachement before drawing it
	colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//What to do with stencil before rendering
	colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	//What to do with stencil after rendering

	//Frame Buffer dataq will be stored as image, but images will be given different data layouts
	//to give optimal use for certain operation (e.g read only througth shader or present them to screee)
	colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			 // Image data layout before render pass starts
	//subpassLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ->later colorAttachmentRefference layout will be used here
	colorAttachement.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	//It will be read AND written to therefore not READD ONLY     //Image data layout after render pass(to change to)
	
	//Attachment refference in  VkSubpassDescription uses an attachment index 
	//that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference referenceColorAttachment = {}; //if have another subpass can reuse tis attachment refference
	referenceColorAttachment.attachment = 1; //This is the inex of list of attacmnets
	referenceColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	/*
		What to do when try two refferences to be used by one subpass. Try ->later
	*/
	//Information about particular subpass Render Pass is using

	//Attachement -> Depth Attachment
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = chooseSupportedImageFormat(formats,
		VK_IMAGE_TILING_OPTIMAL, { VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT });
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //! DONT NEED TO STORE BEFORE DRAWING
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  //TODO CHEcK official site to see better performance depth stuff
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // DEPTH IS A stencil in a way

	//Refference Depth Attachment
	VkAttachmentReference refferenceDeptAttachment = {};
	refferenceDeptAttachment.attachment = 2; //index in attachments array
	refferenceDeptAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].pInputAttachments = nullptr;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &referenceColorAttachment;
	subpasses[0].pDepthStencilAttachment = &refferenceDeptAttachment;


	//Attachment & Reference for second subpass -> subpass[1]
	//subpasses[1] -> col_final, col_1(diff layout), depth()
	VkAttachmentDescription swapImageColorAttachment = {};
	swapImageColorAttachment.format = swapChainImageFormat;						//Format to use for attachement
	swapImageColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					//Number Of samples to write for multsisampling
	swapImageColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				//What to do with attachement before rendering
																				//storeOp do not care -> used ussually for the depth buffer
	swapImageColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			//What to do with attachement after rendering it Store atachement before drawing it
	swapImageColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//What to do with stencil before rendering
	swapImageColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	//What to do with stencil after rendering

																		//Frame Buffer dataq will be stored as image, but images will be given different data layouts
																		//to give optimal use for certain operation (e.g read only througth shader or present them to screee)
	swapImageColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			 // Image data layout before render pass starts																	 //subpassLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ->later colorAttachmentRefference layout will be used here
	swapImageColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	     //Image data layout after render pass(to change to)

																		 //Attachment refference in  VkSubpassDescription uses an attachment index 
																		 //that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference referenceSwapImageColorAttachment = {}; //if have another subpass can reuse tis attachment refference
	referenceSwapImageColorAttachment.attachment = 0; //This is the inex of list of attacmnets
	referenceSwapImageColorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference refColorFirstToSecondSubpass = {};
	refColorFirstToSecondSubpass.attachment = 1;
	refColorFirstToSecondSubpass.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference refDeptFirstToSecondSubpass = {};
	refDeptFirstToSecondSubpass.attachment = 2;
	refDeptFirstToSecondSubpass.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	std::array<VkAttachmentReference, 2> refInputAttachments{refColorFirstToSecondSubpass, refDeptFirstToSecondSubpass};
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].inputAttachmentCount = static_cast<uint32_t>(refInputAttachments.size());
	subpasses[1].pInputAttachments = refInputAttachments.data();
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &referenceSwapImageColorAttachment;

	//The point in pipeline where subpass will bind -> this is not only graphics,
	//but also computational and sort of transfer pipeline 
	//Need to determine when layout transitions occur using subpass dependancies
	//what subpasses depend on in regard to one another.Also reate implicit layout transitions

	//will use two dependencies
	std::array<VkSubpassDependency, 3> subpassDepedencies;

	//Conversion from VK_IMAGE_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//this is the order -> after /before
	subpassDepedencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;//;						//VK_SUBPASS_EXTERNAL = special value meaning outside of renderpass
	subpassDepedencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;  //Pipeline stage
	subpassDepedencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;			//Stage memory access mask
	
	subpassDepedencies[0].dstSubpass = 0; //is this zero the id of the subpass <- first color/depth subpass[0] 
	subpassDepedencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDepedencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	subpassDepedencies[0].dependencyFlags = 0;



	//Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	subpassDepedencies[1].srcSubpass = 0;			//some says it must be 1	was 1								 //Transition has to happen after our main subpass has done its draw operation 
	subpassDepedencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDepedencies[1].srcAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//was VK_ACCESS_COLOR_ATTACHMENT_READ_BIT

	subpassDepedencies[1].dstSubpass = 1; //is this zero the id of the subpass
	subpassDepedencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDepedencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDepedencies[1].dependencyFlags = 0;



	subpassDepedencies[2].srcSubpass = 0;  //0 at the guy lectures	works with 1											 //Transition has to happen after our main subpass has done its draw operation 
	subpassDepedencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	subpassDepedencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	subpassDepedencies[2].dstSubpass = VK_SUBPASS_EXTERNAL; //is this zero the id of the subpass
	subpassDepedencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;// VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//try with : VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
	subpassDepedencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDepedencies[2].dependencyFlags = 0;

	//Color and Depth attachments ->order is important
	std::array<VkAttachmentDescription, 3> attachments { swapImageColorAttachment, colorAttachement, depthAttachment };


	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size()) ;//was 1 before depth
	renderPassCreateInfo.pAttachments = attachments.data(); //was &colorAttachement
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDepedencies.size());
	renderPassCreateInfo.pDependencies = subpassDepedencies.data();

	VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create render pass");
	}
}

void VulkanRenderer::createFramebuffers()
{
	//create framebuffer for each image in to be drawn througth swapchain 
	swapChainFramebuffers.resize(swapChainImages.size());

	for(size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 3> attachments = {
			swapChainImages[i].imageView , colorImageView[i], depthImageView[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;								//RenderPass layout the frame buffer will be used with
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();			//List of attachments !! 1to1 !!
		//pAttachments -> real attachments, not the dwscriptiuons in renderpass
		//pAttachments is/are in vector <swapChainImage> swapChainImages;
		//swapChainImage ->custom struct with image and imageview
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //imageVIew can look at multiple layer of an image; -> swapChainInfo.imageArrayLayers
		
		VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create Framebuffer");
		}
	}
}

void VulkanRenderer::createUniDescriptorSetBuffers()
{
	//Uniform
	descriptorBuffersSta.resize(swapChainImages.size());
	descriptorsMemorySta.resize(swapChainImages.size());

	/*
	//Dynamic 
	descriptorBuffersDyn.resize(swapChainImages.size());
	descriptorsMemoryDyn.resize(swapChainImages.size());
	*/

	VkDeviceSize uniBuffSize = sizeof(Camera::PV);
	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		//Stasic
		createBufferAndAlloc(mainDevice.physicalDevice, mainDevice.logicalDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniBuffSize, &descriptorBuffersSta[i],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &descriptorsMemorySta[i]);
	
		/*
		//Dynamic
		createBufferAndAlloc(mainDevice.physicalDevice, mainDevice.logicalDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignedSpacePerModelSize*MAX_OBJECTS, &descriptorBuffersDyn[i],
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &descriptorsMemoryDyn[i]);
		*/
	}
	
	//map into update function

}

void VulkanRenderer::createUniDescriptorsPool()
{
	//TODO: may break here
	//Subpool Static
	VkDescriptorPoolSize subPoolsSta = {};
	subPoolsSta.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	subPoolsSta.descriptorCount = (uint32_t)swapChainImages.size();//total num of descriptors for all images
	//descriptor != descriptor set
	

	//!!Commented everything Dyn desc  because do not use it(push const instead)
	/*
	VkDescriptorPoolSize subPoolsDyn = {};
	subPoolsDyn.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	subPoolsDyn.descriptorCount = (uint32_t)(swapChainImages.size());// TODO why not swapChainImages.size()*MAX_OBJECTS

	*/

	std::vector<VkDescriptorPoolSize> poosSizes{ subPoolsSta }; //subPoolsDyn 
	 

	VkDescriptorPoolCreateInfo descPoolCreateInfo = {};
	descPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolCreateInfo.maxSets = (uint32_t)swapChainImages.size(); //Sets for whole shit  
	descPoolCreateInfo.poolSizeCount = (uint32_t)poosSizes.size();
	descPoolCreateInfo.pPoolSizes = poosSizes.data();

	VkResult result =  vkCreateDescriptorPool(mainDevice.logicalDevice, &descPoolCreateInfo, nullptr, &descriptorPool);
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Uniform Descriptor Pool");
	}
}

void VulkanRenderer::createUniDescriptorSets()
{
	//One set for every buffer
	descriptorSets.resize(swapChainImages.size());

	std::vector<VkDescriptorSetLayout> descLayoutsVec;
	descLayoutsVec.resize(descriptorSets.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo descSetAllocateInfo = {};
	descSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocateInfo.descriptorPool = descriptorPool;

	descSetAllocateInfo.descriptorSetCount = (uint32_t)(descLayoutsVec.size());
	descSetAllocateInfo.pSetLayouts = descLayoutsVec.data(); //!!1:1 layout  for every set


	VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &descSetAllocateInfo, descriptorSets.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Allocate Descriptor Sets");
	}
	
	//UPDATE BINDING AND BUFFER CONNECTIONS FOR DESCRIPTOR SET
		//one set per image  => numImages == numSets
	VkWriteDescriptorSet descWritesDyn = {};
	VkWriteDescriptorSet descWritesSta = {};
	VkDescriptorBufferInfo descBuffInfoSta = {};
	VkDescriptorBufferInfo descBuffInfoDyn{};
	for (size_t i = 0; i < descriptorSets.size(); i++)
	{		
	
		descBuffInfoSta.buffer = descriptorBuffersSta[i];
		descBuffInfoSta.offset = 0;
		descBuffInfoSta.range = sizeof(Camera::PV);

		//Data about connection between set and buffer 
		descWritesSta.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWritesSta.dstSet = descriptorSets[i];
		descWritesSta.dstBinding = 0;     //binding in .vert file
		descWritesSta.dstArrayElement = 0;
		descWritesSta.descriptorCount = 1;
		descWritesSta.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWritesSta.pImageInfo = nullptr;
		descWritesSta.pBufferInfo = &descBuffInfoSta;


		/*
		descBuffInfoDyn.buffer = descriptorBuffersDyn[i];
		descBuffInfoDyn.offset = 0;
		descBuffInfoDyn.range = alignedSpacePerModelSize;// Just the range of one idividual descriptor *MAX_OBJECTS;

		
		
		descWritesDyn.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWritesDyn.dstSet = descriptorSets[i];
		descWritesDyn.dstBinding = 1;
		descWritesDyn.dstArrayElement = 0;
		descWritesDyn.descriptorCount = 1;//orig 1
		descWritesDyn.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descWritesDyn.pImageInfo = nullptr;
		descWritesDyn.pBufferInfo = &descBuffInfoDyn;
		*/
		std::vector<VkWriteDescriptorSet> descSetWritesStaDyn;
		//descSetWritesStaDyn.push_back(descWritesDyn);
		descSetWritesStaDyn.push_back(descWritesSta); 

		vkUpdateDescriptorSets(mainDevice.logicalDevice, (uint32_t)descSetWritesStaDyn.size(), descSetWritesStaDyn.data(), 0, nullptr);
	}
	 //NEXT STEP UPDATE RECORD COMMANDS
}



void VulkanRenderer::loadTexName(std::string fileName)
{
	textureNamesPool.push_back(fileName);
}

void VulkanRenderer::createTexFromTexPool() 
{
	textureImages.resize(textureNamesPool.size());
	textureImagesMemory.resize(textureNamesPool.size());
	textureImageViews.resize(textureNamesPool.size());

	for (int i = 0; i < textureNamesPool.size(); i++)
	{
		createTextureImage(i);
		textureImageViews[i] = createImageView(textureImages[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureDesc(textureImageViews[i]);
	}
}
	

void VulkanRenderer::createTextureImage(int pos)
{
	int width;
	int height;
	stbi_uc* rawTextureImage = loadTextureFile(textureNamesPool[pos], &width, &height, &loadedTextureSize);//fileName

	//Create Staging Buffer
	createBufferAndAlloc(mainDevice.physicalDevice, mainDevice.logicalDevice,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, loadedTextureSize, &rawTexStagingBuffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &rawTexStagingBufferMemory
	);

	//Map Memory
	void* data;
	vkMapMemory(mainDevice.logicalDevice, rawTexStagingBufferMemory, 0, loadedTextureSize, 0, &data);
	memcpy(data, rawTextureImage, static_cast<size_t>(loadedTextureSize));
	vkUnmapMemory(mainDevice.logicalDevice, rawTexStagingBufferMemory);

	//VkImage tmpImage; replace with pos

	// VkDeviceMemory tmpImageMemory;replace with pos
	textureImagesMemory[pos];
	VkExtent2D texDimensionsPixels;
	texDimensionsPixels.width = static_cast<uint32_t>(width);
	texDimensionsPixels.height = static_cast<uint32_t>(height);


	textureImages[pos] = createImage(VK_FORMAT_R8G8B8A8_UNORM, texDimensionsPixels, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&textureImagesMemory[pos], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//Transition image layout from undefined to transfer_dst_optimal
	transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		textureImages[pos], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//Fill the tmpImage with data from the staging buffer
	copyBufferToImage(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		rawTexStagingBuffer, textureImages[pos], width, height, loadedTextureSize);

	//Transition image layout to shader_read_only_optimal for later use
	transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		textureImages[pos], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//Free
	stbi_image_free(rawTextureImage);
	//Copy raw data to texture Image and shange l

	vkDestroyBuffer(mainDevice.logicalDevice, rawTexStagingBuffer, nullptr);
	vkFreeMemory(mainDevice.logicalDevice, rawTexStagingBufferMemory, nullptr);

	
}


VkSampler VulkanRenderer::createTextureSampler()
{
	VkSampler textureSampler;

	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE; //TODO: fix enabed in phys props
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f; //max level of detail to pick mipmap()we will not use mipmap
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult result = vkCreateSampler(mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
	
	if(result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Sampler");
	}
	return textureSampler;
}

void VulkanRenderer::createSamplerDescriptorPool()
{

	VkDescriptorPoolSize samplerPoolSize;
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount =(uint32_t) MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = (uint32_t) MAX_OBJECTS; //one set per object
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &samplerPoolCreateInfo, nullptr, &TexturesDescPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create Sampler Descriptor Pool");
	}

}

int VulkanRenderer::createTextureDesc(VkImageView textureImageView)
{
	VkDescriptorSet textureDescSet;

	VkDescriptorSetAllocateInfo textureDescAllocInfo = {};
	textureDescAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	textureDescAllocInfo.descriptorPool = TexturesDescPool;
	textureDescAllocInfo.descriptorSetCount = 1;
	textureDescAllocInfo.pSetLayouts = &samplerDescLayout;

	vkAllocateDescriptorSets(mainDevice.logicalDevice, &textureDescAllocInfo, &textureDescSet);

	VkDescriptorImageInfo teximageInfo = {};
	teximageInfo.sampler = sampler;
	teximageInfo.imageView = textureImageView;
	teximageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkWriteDescriptorSet texDescWrite = {};
	texDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texDescWrite.dstSet = textureDescSet;
	texDescWrite.dstBinding = 0;
	texDescWrite.dstArrayElement = 0;
	texDescWrite.descriptorCount = 1;
	texDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texDescWrite.pImageInfo = &teximageInfo;
	texDescWrite.pBufferInfo = nullptr;			//we dont bind it to buffer;
	texDescWrite.pTexelBufferView = nullptr;	// do not use Texel

	

	vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &texDescWrite, 0, nullptr);

	TexturesDescSets.push_back(textureDescSet);

	return TexturesDescSets.size() - 1;
}


void VulkanRenderer::createColorDepthDescriptorPool()
{


	VkDescriptorPoolSize colorPoolSize = {};
	colorPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	colorPoolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());//one descriptor colDepth for all the images

	VkDescriptorPoolSize depthPoolSize = {};
	depthPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthPoolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());//one descriptor colDepth for all the images

	std::array<VkDescriptorPoolSize, 2> colorDepthPoolSize{ colorPoolSize, depthPoolSize };
	VkDescriptorPoolCreateInfo colorDepthPoolCreateinfo = {};
	colorDepthPoolCreateinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	colorDepthPoolCreateinfo.maxSets = static_cast<uint32_t>(swapChainImages.size()); 
	colorDepthPoolCreateinfo.poolSizeCount = static_cast<uint32_t>(colorDepthPoolSize.size());
	colorDepthPoolCreateinfo.pPoolSizes = colorDepthPoolSize.data();


	VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &colorDepthPoolCreateinfo, nullptr, &colorDepthDescPool);
	if (result != VK_SUCCESS)
	{

		throw std::runtime_error("Failed to Create COLDEPTH pool");
	}
}

void VulkanRenderer::createColorDepthDescSets()
{
	std::vector<VkDescriptorSetLayout> colorDepthDescLayouts;
	colorDepthDescLayouts.resize(swapChainImages.size(), colorDepthDescLayout);
	colorDepthDescSets.resize(swapChainImages.size());

	VkDescriptorSetAllocateInfo colorDepthAllocInfo = {};
	colorDepthAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	colorDepthAllocInfo.descriptorPool = colorDepthDescPool;
	colorDepthAllocInfo.descriptorSetCount = static_cast<uint32_t>(colorDepthDescSets.size());
	colorDepthAllocInfo.pSetLayouts = colorDepthDescLayouts.data();

	VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &colorDepthAllocInfo, colorDepthDescSets.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Alloc Input CollorDept Desriptor Sets");
	}
	
	for (size_t i = 0; i < colorDepthDescSets.size(); i++)
	{

		VkDescriptorImageInfo colorInputImageInfo = {};
		colorInputImageInfo.sampler = VK_NULL_HANDLE;
		colorInputImageInfo.imageView = colorImageView[i];
		//NOT VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		colorInputImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Layout of image when being read, read in the second subpass

		VkWriteDescriptorSet colorDescriptorWrite = {};
		colorDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		colorDescriptorWrite.dstSet = colorDepthDescSets[i];
		colorDescriptorWrite.dstBinding = 0;
		colorDescriptorWrite.dstArrayElement = 0;
		colorDescriptorWrite.descriptorCount = 1;
		colorDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorDescriptorWrite.pImageInfo = &colorInputImageInfo;
		colorDescriptorWrite.pBufferInfo = nullptr;
		colorDescriptorWrite.pTexelBufferView = nullptr;

		VkDescriptorImageInfo depthInputImageInfo = {};
		depthInputImageInfo.sampler = VK_NULL_HANDLE;
		depthInputImageInfo.imageView = depthImageView[i];
		depthInputImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet depthDescriptorWrite = {};
		depthDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depthDescriptorWrite.dstSet = colorDepthDescSets[i];
		depthDescriptorWrite.dstBinding = 1;
		depthDescriptorWrite.dstArrayElement = 0;
		depthDescriptorWrite.descriptorCount = 1;
		depthDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthDescriptorWrite.pImageInfo = &depthInputImageInfo;
		depthDescriptorWrite.pBufferInfo = nullptr;
		depthDescriptorWrite.pTexelBufferView = nullptr;

		std::array<VkWriteDescriptorSet, 2 > colorDepthWrites{ colorDescriptorWrite , depthDescriptorWrite };

		vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(colorDepthWrites.size()), colorDepthWrites.data(), 0, nullptr);
	}
}


void VulkanRenderer::createCommandPool()
{

	//Get indices  of queue families, save later
	QueueFamilyIndicies queueFamiliIndices = getQueFamilyIndices(mainDevice.physicalDevice);

	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = (uint32_t)queueFamiliIndices.graphicsFamily;

	//Create Graphics Queue Family Command Pool
	VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &graphicsCommandPool);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create a Command Pool");
	}

}
void VulkanRenderer::createCommandBuffers()
{
	//resize command bufferS to have one for each framebuffer
	commandBuffers.resize(swapChainFramebuffers.size());//
	
	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = graphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; /*VK_COMMAND_BUFFER_LEVEL_PRIMARY: Buffer you submit directly to queue. Cant be called by other buffers
														   VK_COMMAND_BUFFER_LEVEL_SECONDARY: Buffer cant be called directly. Can be called from other buffers via "vkCmdExecuteCommands when recording commands in primary buffer"*/
	
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	
	VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice,
						&cbAllocInfo, commandBuffers.data()); //Allocate multiple command buffers froma our pool and places handles into arrays of  buffers -> commandBuffers
	if(result!=VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Allocate Command Buffers!");
	}
}
void VulkanRenderer::createSynchronisation()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_DRAWS);
	renderFinishedSemaphores.resize(MAX_FRAMES_DRAWS);
	drawFences.resize(MAX_FRAMES_DRAWS);
	//Semaphore creation information
	VkSemaphoreCreateInfo semaphoreCreateInfo = {} ;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO; 

	//Fence creation information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (int i =0; i < MAX_FRAMES_DRAWS; i++)
	{
		if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
			|| vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
			|| vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create Semaphores");
		}


	}
	
}

void VulkanRenderer::initDebug()
{
	
	pfvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	pfvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	//vkCreateDebugReportCallbackEXT

	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugCreateInfo.pfnCallback = VulkanDebugCallBack;
	debugCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_ERROR_BIT_EXT
		| VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	if (nullptr == pfvkCreateDebugReportCallbackEXT || nullptr == pfvkDestroyDebugReportCallbackEXT)
	{
		throw std::runtime_error("Failed to Assign Debug pFuncs");
	}
	
	
	


	pfvkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &pDebugReport);
}

 VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::VulkanDebugCallBack(VkDebugReportFlagsEXT msgFlags,
	VkDebugReportObjectTypeEXT objType,//obj that caused the error
	uint64_t srcObj,				   //obj addr
	size_t location,				   //??
	int32_t msgCode, 
	const char * layerPrefix,			
	const char * message,
	void * userData)
{
	 std::ostringstream stream;
	 /*
	 if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	 {
		 stream << "INFO : ";
	 }
	 if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	 {
		 stream << "WARNING : ";
	 }
	 if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	 {
		 stream << "PERFORMANCE : ";
	 }
	 */
	 if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	 {
		 stream << "ERROR : ";
	 }
	 /*
	 if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	 {
		 stream << "REPORT : ";
	 }
	 */
	 stream << "@[" << message << "]:";
	 stream << msgFlags << std::endl;

	 //std::cout << stream.str();

#ifdef _WIN32
	 if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	 {
		 MessageBox(NULL, stream.str().c_str(), "Vulkan Error", 0);
	 }
	
#endif
	return false;
}

 void VulkanRenderer::stopDebug()
 {
	 pfvkDestroyDebugReportCallbackEXT(instance, pDebugReport, nullptr);
	 pDebugReport = nullptr;
 }

stbi_uc * loadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize) //VulkanRenderer::
{
	int channels;
	std::string filePath = "Textures/" + fileName;
	stbi_uc* imageRaw = stbi_load(filePath.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!imageRaw)
	{
		throw std::runtime_error("Failed To Load Texture File (" + fileName + ")");
	}

	*imageSize = (*width) * (*height) * 4;
	return imageRaw;
}
void VulkanRenderer::initPushConstRange()
{
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  //SHADER_STAGE NOT PIPELINE
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushModel);
}
void VulkanRenderer::updateUniDescSets(uint32_t frameIndex)
{
	
	VkDeviceSize buffSize = sizeof(Camera::PV);
		void* data; //->this is dst whish is mapped to somhere else througth vkMapMempry
		vkMapMemory(mainDevice.logicalDevice, descriptorsMemorySta[frameIndex],	0, buffSize, 0, &data);
		memcpy(data,&camera.pv , (size_t)buffSize);//&pv
		vkUnmapMemory(mainDevice.logicalDevice, descriptorsMemorySta[frameIndex]);


		/* 
		//Not needed anymore because will use pus const instead dyn uniform buffer 
		for (size_t i = 0; i < meshList.size(); i++)
		{
			UboModelStruct* thisModel = (UboModelStruct*)((uint64_t)ptrDynBuffTransferSpace + (i* alignedSpacePerModelSize));
			*thisModel = meshList[i].getPushConstModelStr();
		}
		vkMapMemory(mainDevice.logicalDevice, descriptorsMemoryDyn[frameIndex],
			0, alignedSpacePerModelSize*meshList.size(), 0, &data);
		memcpy(data, ptrDynBuffTransferSpace, alignedSpacePerModelSize*meshList.size());
		vkUnmapMemory(mainDevice.logicalDevice, descriptorsMemoryDyn[frameIndex]); 
		*/


		/*
		TODO this is mine will try wit it later
		UboModelStruct* offset = ptrDynBuffTransferSpace; 
		for(size_t i = 0; i < MAX_OBJECTS; i++)
		{
		*offset = *(UboModelStruct*)(ptrDynBuffTransferSpace + (i*alignedSpacePerModelSize));

			vkMapMemory(mainDevice.logicalDevice, descriptorsMemoryDyn[frameIndex],
					i*alignedSpacePerModelSize,alignedSpacePerModelSize, 0,&data);
			memcpy(data, offset, alignedSpacePerModelSize);
		}
		*/
		
}
 void VulkanRenderer::getMinimalAlignmentAvailable(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	
	//min available mem for dynamic discriptor{model}//gpu specific 
	minAligmentAvailable = properties.limits.minUniformBufferOffsetAlignment;
}
 void VulkanRenderer::allocDynBuffTransferSpace()
 {
	 alignedSpacePerModelSize = (sizeof(UboModelStruct) + minAligmentAvailable - 1) //modelSize + minAll-1
						 & ~(minAligmentAvailable - 1);

	 //create space in mem to hold dyn buffer
	 ptrDynBuffTransferSpace = (UboModelStruct*)_aligned_malloc(MAX_OBJECTS*alignedSpacePerModelSize, alignedSpacePerModelSize);
 //TODO try with  (VkDeviceSize*)ptrDynBuffTransferSpace = (VkDeviceSize*)_aligned_malloc(MAX_OBJECTS*alignedSpacePerModelSize, alignedSpacePerModelSize);
 }

 
void VulkanRenderer::updateModel(int modelID,glm::mat4 newModelMatrix) //Mesh* meshObj
{
	//if (modelID >= meshList.size()) //TODO make meshlist mem var // TODO: write in memory designated to model
	if(modelID >= meshModels.size())
	{
		throw	std::runtime_error("Modeldoes not exist");
	}
	for (int i = 0; i < meshModels[modelID].getMeshListSize(); i++)
	{
		meshModels[modelID].getMesh(i)->setModelMatrix(newModelMatrix);
	}
	
}

MeshModel * VulkanRenderer::getModel(int modelID)
{
	
	return &meshModels[modelID];
}

void VulkanRenderer::getPhysicalDevice()
{
	//enumurate physical devices The Vk Instance can access;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	//
	if(deviceCount ==0)
	{
		throw std::runtime_error("Can not find  Vulka supporting GPU");
	}

	//
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
	

	//to do add devices
	for(auto & physicalDevice : physicalDevices)
	{
		if(checkDeviceSuitable(physicalDevice))
		{
			mainDevice.physicalDevice = physicalDevice;
			break;
		}
	}
}

void VulkanRenderer::recordCommands(uint32_t currentFrame)
{
	//Information how to begin each command buffer
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //Buffer can be resubmitted if it has already been submited and is awaiting execution
	
	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };	//Start point of render pass in 
	renderPassBeginInfo.renderArea.extent = swapChainExtent;
	
	std::array<VkClearValue, 3 > clearValues = {};		//2 clear values becuse we have two attachments in render pass
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f };	// 
	clearValues[1].color = { 0.6f, 0.65f, 0.4f, 1.0f }; //[0] index of first attachment in render pass->col attachment
	clearValues[2].depthStencil.depth = 1.0f;			//
	
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	//will be update for current frame instead for all swapChainImages
	renderPassBeginInfo.framebuffer = swapChainFramebuffers[currentFrame];
	//Start recording commands to command buffer	
    VkResult result = vkBeginCommandBuffer(commandBuffers[currentFrame], &bufferBeginInfo);
	if (result != VK_SUCCESS)	
	{	
		throw std::runtime_error("Failed to Start a Command Buffer");	
	}	
		
	//can use compute, transfer-> memory or graphics	
	//Begin render pass	
	vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //VK_S	UBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS means same render pass will be used in secondary comand buffer 
	
	//Bind Pipeline to be used in render pass//Can bind more pipelines here	
	vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			//TODO add model vec size	
		for (size_t j = 0; j < meshModels[0].getMeshListSize(); j++)
		{
			Mesh* pMesh = meshModels[0].getMesh(j);
			
			VkBuffer vertexBuffers[] = { *pMesh->getVertexBuffer() };	/* */				//Buffers to bind
			
			VkDeviceSize offsets[] = { 0 };		

			vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, pMesh->getVertexBuffer(), offsets);	//Binds vertex buffer before drawing with it
			
			vkCmdBindIndexBuffer(commandBuffers[currentFrame],pMesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32); /* meshList[j].getIndexBuffer() */

			vkCmdPushConstants(commandBuffers[currentFrame], 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT,
				0, 
				sizeof(PushModel), /*glm::mat4*/
				&meshModels[0].getModelMatrix()); /*&meshList[j].getPushConstModelStr()*/

			std::array<VkDescriptorSet, 2> descriptorsGroup { descriptorSets[currentFrame],//view and projection for every frame
				TexturesDescSets[pMesh->getTexID()] }; //texture for every mesh
			

			//uint32_t dynamicOffsets = static_cast<uint32_t>(alignedSpacePerModelSize * j);
			vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0,/*set in vert file we want to start with*/
				static_cast<uint32_t>(descriptorsGroup.size()),/*size() array of sets we will use )*/
				descriptorsGroup.data(),
				0, // !!! we are using ONE offset. ONE dynamic descriptor // 1
				nullptr);  //dyn descs  starting point// &dynamicOffsets
			/*first binding is the invisible binding = 0 in vert file
			binding happens in VkPipelineVertexInputStateCreateInfo bindingDescription = {}; in graphicsPipeline
			*/

			//Execute pipeline should put vkbindvertices somewhere ??vkCmdBindVertexBuffers??//1-> instance of the object() draw same object several times)
			/* REPLACE vkCmdDraw with vkCmdDrawIndexed in order to draw the indices
			vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(firstMesh.getVerticesCount()), 1, 0, 0 ); //static_cast<uint32_t>(firstMesh.getVerticesCount()      static_cast<uint32_t>(firstMesh.getVerticesCount())
			*/

			
			
			vkCmdDrawIndexed(commandBuffers[currentFrame], pMesh->getIndcesCount(), 1, 0, 0, 0);
			
			//This is New wasnt here
			//vkDestroyBuffer(mainDevice.logicalDevice, vertexBuffers[0], nullptr);
		
		}
		//Offsets  into buffers being bound
		
		//Start Next Subpass
		vkCmdNextSubpass(commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipeline);
		vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipeLayout,
			0,/*set in vert file we want to start with*/
			1,/*size() array of sets we will use )*/
			&colorDepthDescSets[currentFrame],
			0, // !!! we are using ONE offset. ONE dynamic descriptor // 1
			nullptr);  //dyn descs  starting point// &dynamicOff
		vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
		//instance index can be found(shaderver? or in place and place ofset for every instance in order to place at diff locations)
		//Cmd -> command that can be recorded
		//End render pass
		vkCmdEndRenderPass(commandBuffers[currentFrame]);

		//Stop recording commands to command buffer
		result = vkEndCommandBuffer(commandBuffers[currentFrame]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to stop a Command Buffer");
		}
	//}

		
}

void VulkanRenderer::transitionImageLayout(VkDevice device, VkQueue queue,
 VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer tmpCommandBuffer = createAndBeginTmpCommandBuffer(device, queue, commandPool);

	VkImageMemoryBarrier imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.oldLayout = oldLayout;
	imageBarrier.newLayout = newLayout;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.subresourceRange.levelCount = 1;
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if(imageBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED 
		&& imageBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)		//the Layout used in vkCmdCopyBufferToImage()
	{
		imageBarrier.srcAccessMask = 0;											//thransition must happen aftet -> whenever
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;				//thransition must happen before -> transfer from rawBuff to VkImage
	
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (imageBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		&& imageBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)	//the Layout needed in order to use the texture
	{
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	
	vkCmdPipelineBarrier(tmpCommandBuffer, srcStage, dstStage,
		0,																		//no dependencyFlags
		0, nullptr,
		0, nullptr,
		1, &imageBarrier);

	EndAndSubmitTmpCommandBuffer(device, &tmpCommandBuffer, graphicsQueue, graphicsCommandPool);
}

void VulkanRenderer::mapTexNamesToTexIDs(std::vector<std::string> &texNamesPool, modelMaterials& material)
{
	//texIDsPool.resize(texNamesPool.size()); //will do with tex names pool and texture names

	//fill indice vector of not empty textures
	//and find number of texture images needed

	for (int i = 0; i < material.modelTexturesFileNames.size(); ++i)//
	{
		updateIDs(material.modelTexturesFileNames[i], i, texNamesPool, material);
	}
	material.availableTexturesCount = numTextures + 1;
	numTextures = 0;
/*


		if (texNamesPool[i].empty())
		{
			modelTex.modelTexturesIDs[i] = placeholdersIDsInPool[0]; //TODO add check make it better
		}
		else 
		{
			modelTex.modelTexturesIDs[i] = getTexIDFromPool(modelTex.modelTexturesNames[i]);
		}
	}

	textureImages.resize(numTextureImages + placeholdersTextureNames.size());
	textureImageViews.resize(numTextureImages + placeholdersTextureNames.size());
	textureNamesPool.insert(textureNamesPool.end(), placeholdersTextureNames.begin(), placeholdersTextureNames.end());
	*/
}


void VulkanRenderer::updateIDs(std::string str,int pos, std::vector<std::string>& pool, modelMaterials& modelTex)
{
	
	int j = 0;
	bool found = false;
	for (j; j < pool.size(); j++)
	{
		std::string pool_j = pool[j];
		bool equal = str == pool[j];
		bool equo_is_0 = equal == 0;
		if (str == pool[j])  
		{
			found = true;
			modelTex.modelTexturesIDs[pos] = j;
			break;
		}
			
	}
	if (found == false)
	{
		pool.push_back(str);
		modelTex.modelTexturesIDs[pos] = numTextures;
		numTextures++;
	}

	
}

void VulkanRenderer::createMeshModel(std::string fileName, std::vector<MeshModel> &newMeshModelList)
{
	Assimp::Importer importer; 
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	 modelsMaterialsList.emplace_back(MeshModel::LoadTexNamesForMats(scene));//TODO think of better name, split function maybe
		   
	 mapTexNamesToTexIDs(textureNamesPool,  modelsMaterialsList.back());
	
	 std::vector<Mesh>modelMeshList;
	 modelMeshList = MeshModel::LoadNode(mainDevice.physicalDevice, mainDevice.logicalDevice,
		 graphicsQueue, graphicsCommandPool,
		 scene,
		 scene->mRootNode,
		 modelsMaterialsList.back().modelTexturesIDs,
		 modelsMaterialsList.back());//textureIDsPool
	
	 std::ofstream fileMeshNames;
	 fileMeshNames.open("meshNames.txt");
	 
	
	 for (int i = 0; i < scene->mNumMeshes; i++)
	 {
		std::string s = (scene->mMeshes[i]->mName).data;
		fileMeshNames << s << '\n';
	 }
	 fileMeshNames.close();
	 /*
	 //New
	 //Create Textures
	 //TODO Fix it
	 textureImages.resize(textureNamesPool.size());
	 textureImagesMemory.resize(textureNamesPool.size()); 
	 textureImageViews.resize(textureNamesPool.size());
	 //resize sampler descriptor set later
	 //
	 for(int i = 0; i <  textureNamesPool.size(); i++ )
	 {
		 createTexFromTexPool(i);
	 }
	 */
	 newMeshModelList.emplace_back(MeshModel(modelMeshList));
	 newMeshModelList.back().setModelID(meshModels.size() - 1);
}


VkImage VulkanRenderer::createImage( VkFormat format, VkExtent2D extent2d, VkImageTiling tiling, VkImageUsageFlags usageFlag, VkDeviceMemory* depthMem, VkMemoryPropertyFlags memoryProperties)
	{
		/*
		VkExtent3D extent = {};
		extent.depth = 0;
		extent.height = extent2d.height;
		extent.width = extent2d.width;
		*/
		//QueueFamilyIndicies indicies = getQueFamilyIndices(mainDevice.physicalDevice); //this one runs
		//uint32_t graphicsFamInd = static_cast<uint32_t>(indicies.graphicsFamily);

		 //Create Image

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent.height = extent2d.height;
		imageCreateInfo.extent.width = extent2d.width;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usageFlag;//depth image ->VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	
		//imageCreateInfo.queueFamilyIndexCount = 1;
		//imageCreateInfo.pQueueFamilyIndices = &graphicsFamInd;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage image;
		VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo, nullptr, &image);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create Image");
		}

		
		//Allocate Memory

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mainDevice.logicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.allocationSize = memRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice.physicalDevice,
			memRequirements.memoryTypeBits, memoryProperties);

		result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocateInfo, nullptr, depthMem);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Allocate Image Memory");
		}

		//Connect depth image header with depth memory
		 result = vkBindImageMemory(mainDevice.logicalDevice, image, *depthMem, 0);
		 if(result != VK_SUCCESS)
		 {
			 throw std::runtime_error("Failed to Bind Memory");
		 }
		return image;
	}


bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char*>* extensionsToCheck)
{
	//need to get number of extensions to create array
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	//create a list of vkExtensionProperties using count
	std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
	
	//check if given extensions are in list of available extensions
	for (const auto &extensionToCheck : *extensionsToCheck)
	{
		bool hasExtension = false ; 
		for (auto &supportedExtension : supportedExtensions)
		{
			if (strcmp(supportedExtension.extensionName, extensionToCheck))
				hasExtension = true;
			break; 
		}

		if(!hasExtension)
		{
			return false;
		}
	}
	return true;
}

bool VulkanRenderer::checkDeviceExtensionsSupport(VkPhysicalDevice physicalDevice)
{
	//used for SWAPCHAIN  
	uint32_t extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice,nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> retrievedExtensions(extensionsCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, retrievedExtensions.data());
	
	if (extensionsCount == 0)
	{
		return false;
	}
	 
	bool hasExtension = false;
	int countRequiredExtensions = deviceExtensions.size();
	int countMatchedExtensions = 0;
	for (const auto & deviceExtension : deviceExtensions) //deviceExtensions from utilities
	{
		for (const auto & retrievedExtension : retrievedExtensions)
		{		
			if (std::strcmp(deviceExtension, retrievedExtension.extensionName) == 0)
			{
				countMatchedExtensions++;
			}
			if(countMatchedExtensions == countRequiredExtensions)
			{
				hasExtension = true;
				goto CHECK_DONE;
			}				
		}
	}

	CHECK_DONE:	
	return hasExtension;
}

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	/*
	//information about device itself
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(*physicalDevice, &deviceProperties);

	//information about what device can do (geo shader tess shader, widelinesw etc. )
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(*physicalDevice, &deviceFeatures);
	return false;

	*/

	QueueFamilyIndicies indicies = getQueFamilyIndices(physicalDevice);
	bool hasExtesionSupoprted = checkDeviceExtensionsSupport(physicalDevice);
	bool inicesValid = indicies.isValid();
	
	SwapChainDetails swapChainDetails = getSwapChainDatails(physicalDevice);
	bool swapChainValid = false;
	swapChainValid = !swapChainDetails.listOfPresentMode.empty()
		&& !swapChainDetails.listOfSurfaceFormat.empty();
						
	
	return indicies.isValid() && hasExtesionSupoprted && swapChainValid;
}

VkFormat VulkanRenderer::chooseSupportedImageFormat(std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	VkFormatProperties formatProperties;
	
	
	for(auto format : formats)
	{
		vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format, &formatProperties);
		if ((tiling == VK_IMAGE_TILING_LINEAR) &&
			((formatProperties.linearTilingFeatures & features) == features))
		{
			return format;
		}
		else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
			((formatProperties.optimalTilingFeatures & features) == features))
		{

			return format;
		}
	}
}

VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats)
{
	//FORMAT : VK_FORMAT_R8G8B8A8_UNORM
	//COLORSPACE : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; //return some nameless VkSurfaceFormatKHR struct
	}

	for (const auto & format : formats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}	
	}
	return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentMode(std::vector<VkPresentModeKHR>& presentatioModes)
{
	for(const auto presentatioMode : presentatioModes)
	{
		if (presentatioMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentatioMode;
		}
	}	
	//if cant find use fifo
	return VK_PRESENT_MODE_FIFO_KHR;	
}

VkExtent2D VulkanRenderer::chooseSwapExtend(const VkSurfaceCapabilitiesKHR & surfaceCapabilities)
{
	//if current extent is at numeric limits, then extent can vary.Otherwise, it is the size of the wondow
	if (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		//if value can vary we need to set it manualy

		//get window size
		int width, height;
		glfwGetFramebufferSize(window,&width,&height);

		//create new extent using wondows size
		VkExtent2D newExtent{};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		//surface also defines max and min, so make sur within boundaries by clamping value
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.minImageExtent.height, newExtent.height)); 
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		
		return newExtent;
	}	
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo  viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;										 //image to dreate view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					 //type of image(1D,2D,3D Cube, etc)
	viewCreateInfo.format = format;										 //Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;		 //allow remapping of rgba components to other rgb values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	//Subresoureces allow the view to only view a part of view image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;  //which aspect of image to view (e.g COLOR_BIT for view color)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;          //(which levels to view) Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;			   //Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;        //Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;			   //nUMBER OF ARRAY LEVELS TO VIEW

	//Create image view and return it 
	VkImageView imageView;
	VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);//why imageView
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image view");
	}
	return imageView;
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	//use reinterpret_cast to convert pointer type to pointer type
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
	
	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shader Module");
	}
	return shaderModule;
}

QueueFamilyIndicies VulkanRenderer::getQueFamilyIndices(VkPhysicalDevice  physicalDevice)
{
	QueueFamilyIndicies indices;
	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());

	//go through  each que family and check if it has at least one of requred types of queue
	
	int i = 0;
	for(const auto& queueFamily : queueFamilyList)
	{
		//first check if family has at least one queue
		//queue can be multiple types, defined  through bitfield 
		if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) 
		{
			//if que family is in valid  state get index 
			indices.graphicsFamily = i;
		}

		//check if queue family supports presentation
		VkBool32 presentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupport);
		//check if que is both graphics and presentation type
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}
		//if que family is in valid  state stop searching 
		if (indices.isValid())
		{
			break;
		}
		i++;
	}
	return indices;
}

SwapChainDetails VulkanRenderer::getSwapChainDatails(VkPhysicalDevice physicalDevice)
{
	SwapChainDetails swapChainDetails{};

	//capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainDetails.surfaceCapabilities);

	//formats
	uint32_t surfaceFormatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, nullptr);
	if (surfaceFormatsCount != 0)
	{
		swapChainDetails.listOfSurfaceFormat.resize(surfaceFormatsCount);
	}
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, swapChainDetails.listOfSurfaceFormat.data());
	
	//present modes
	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
	if (presentModesCount != 0)
	{
		swapChainDetails.listOfPresentMode.resize(presentModesCount);
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, swapChainDetails.listOfPresentMode.data());
	
	return swapChainDetails;
}
