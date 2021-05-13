#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
class Camera
{
	
public:
	Camera();
	~Camera();
	void updateNear(float newNear); //TODO update prame afteer update in 
	void updateFar(float newFar);
	GLFWwindow * winHandle;
	void keyActions(int key, int action);
	void mouseActions(double widthPos, double heightPos);
	void initHandle();
	void inputCallbacks(GLFWwindow * window, float dTime);// float delta
	
	bool mousedMovedFirstTime = true;
struct PV {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 lightDirection;  //TODO move the light direction out of camera class
}	pv;
void updateView();
//float now;
float deltaTime = 0.0f;
float last = 0.0f;
void update(float deltaT);
private:
float near = 0.1f;
float far = 500.0f;
float speed = 1000.0f;
float turnspeed = 0.1f;
bool flipY = true;
glm::vec3 camPos;
glm::vec3 rotationVec = glm::vec3(0.0f);
glm::vec3 translationVec = glm::vec3();
glm::vec4 viewPos = glm::vec4();
glm::vec3 camTarget;
glm::vec3 camDirection;

struct {
	int forwardState = 0;
	int backwardState = 0;
	int strafeLeftState = 0; //0 up, 1 prssed once, 2 sticky down
	int strafeRightState = 0;
	int lookLeftState = 0;
	int lookRightState = 0;
	int mouseLockedState = 1;
}keys;
glm::vec3 globalUp;
glm::vec3 camRight;
glm::vec3 camUp;
glm::vec3 camFront;


int height;
int width;

double cursorULast;
double cursorVLast;

float yawn = 0;
float pitch = 0;

bool mouseLocked = true;

static void keyCallback(GLFWwindow* window, int key, int code, int action, int mode);
static void cursorPositionCallback(GLFWwindow* window, double width, double ypos);

//input


};

