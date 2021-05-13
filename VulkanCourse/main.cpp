/*
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS

#include<glm/glm.hpp>
#include<glm/mat4x4.hpp>
*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <stdexcept>
#include <vector>
#include <iostream>
#include "VulkanRenderer.h"

GLFWwindow * window;
VulkanRenderer vulkanRenderer;
void initWindow(std::string wName = "Test Window", 
				const int width = 800, const int height = 600)
{
	//init GLFW
	glfwInit();
	//Set GLFW to not work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}



	

double cursorXPos, cursorYPos;

int main()
{
	initWindow();
	
	
	//create vulkan renderer instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE) 
	{
		return EXIT_FAILURE;
	}
	vulkanRenderer.camera.winHandle = window;
	vulkanRenderer.camera.initHandle();

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;




	while(!glfwWindowShouldClose(window))
	{
		
		glfwPollEvents();
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
		vulkanRenderer.camera.inputCallbacks(window, deltaTime);//deltaTime
		//vulkanRenderer.camera.update(deltaTime);
		

		
		/*
		angle += 10.0f * deltaTime;
		if(angle>360 ){	angle -= 360;}
		
		//vulkanRenderer.updateModel(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)));
		glm::mat4 firstModel(1.0f);
		glm::mat4 secondModel(1.0f);

		
		firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -1.9f));
		firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.f, 0.0f, 1.0f));
		
		secondModel =  glm::translate(secondModel, glm::vec3(0.0f, 0.0f, -2.0f));
		secondModel = glm::rotate(secondModel, glm::radians(-angle*2), glm::vec3(0.f, 0.0f, 1.0f));
		
		vulkanRenderer.updateModel(0, firstModel);
		vulkanRenderer.updateModel(1, secondModel);
		*/
		vulkanRenderer.draw();
	}

	vulkanRenderer.cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}