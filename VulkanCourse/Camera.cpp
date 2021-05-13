#include "Camera.h"



Camera::Camera()
{
	
	camPos = glm::vec3(200.f, 0.f , 0.f);
	camTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	camDirection = glm::normalize(camPos - camTarget);
	globalUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camRight = glm::normalize( glm::cross(globalUp, camDirection));
	camFront = -camDirection;
	camUp = glm::cross(camRight, camFront);
	pv.lightDirection = glm::vec3(-0.3f, -0.1f, -2.0f);//glm::normalize(camFront);
	
	
}
void Camera::initHandle()
{
	glfwSetWindowUserPointer(winHandle, this);
	glfwGetFramebufferSize(winHandle, &width, &height);
}
Camera::~Camera()
{
}

void Camera::updateNear(float newNear)
{
	near = newNear;
}

void Camera::updateFar(float newFar)
{
	far = newFar;
}

void Camera::inputCallbacks(GLFWwindow * window,float dTime )//, float delta
{
		
	deltaTime = dTime;
	///GLFWwindow* win = this->window;
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
void Camera::updateView()
{

	pv.view =  glm::lookAt(camPos, camPos + camFront, camUp);
//	std::cout << "post camPos: " << camPos.x << ", " << camPos.y << ", " << camPos.z << ", " << std::endl;
}
void Camera::update(float deltaT)
{


	camFront.x = -glm::cos(glm::radians(rotationVec.x))*glm::cos(glm::radians(rotationVec.y));
	camFront.y = glm::sin(glm::radians(rotationVec.x));
	camFront.z = glm::sin(glm::radians(rotationVec.y))*glm::cos(glm::radians(rotationVec.x));
	camFront = glm::normalize(camFront);

	camRight = glm::normalize(glm::cross(camFront, globalUp));
	camUp = glm::normalize(glm::cross(camRight, camFront));
	
	
	
	if (keys.forwardState != 0)
	{
		camPos += camFront * speed * deltaT;
	}
	if (keys.backwardState != 0)
	{
		camPos -= camFront * speed *  deltaT;
	}
	if (keys.strafeLeftState != 0)
	{
		camPos -= glm::normalize(glm::cross(camFront, camUp)) * speed * deltaT;
	}

	if (keys.strafeRightState !=0)
	{
		camPos += glm::normalize(glm::cross(camFront, camUp))* speed * deltaTime;
	}

	

	updateView();
}

void Camera::keyActions(int key, int action)
{

	if (key == GLFW_KEY_W )
	{
		keys.forwardState = action;
	}

	if (key == GLFW_KEY_S )//&& action == GLFW_PRESS
	{
		keys.backwardState = action;
	}
	if (key == GLFW_KEY_A )// && action == GLFW_PRESS
	{
		keys.strafeLeftState = action;
	}
	if (key == GLFW_KEY_D )//&& action == GLFW_PRESS
	{
		keys.strafeRightState = action;
	}

	if (key == GLFW_KEY_J)
	{
		rotationVec.y -= 0.7;
	}
	if (key == GLFW_KEY_K)
	{
		rotationVec.y += 0.7;
	}
	if (key ==GLFW_KEY_ESCAPE)
	{
		mouseLocked = !mouseLocked;
	//	keys.mouseLockedState = action;

	}
	update(deltaTime);
}


void Camera::mouseActions(double uPos, double vPos)
{
	if (mousedMovedFirstTime)
	{

		cursorULast = uPos;
		cursorVLast = vPos;
		mousedMovedFirstTime = false;
	}
	

		rotationVec.y += (float)(cursorULast - uPos)*turnspeed;
		rotationVec.x += (float)(cursorVLast - vPos)*turnspeed;

		if (rotationVec.x > 89.0f)
			rotationVec.x = 89.0f;
		
		if (rotationVec.x < -89.0f)
			rotationVec.x = -89.0f;
		
		update(deltaTime);
		if (mouseLocked) //keys.mouseLockedState !
		{
			//glfwSetCursorPos(winHandle, 800.0/2.0, 600.0/2.0);
			cursorULast = 800.0 / 2.0;
			cursorVLast = 600.0 / 2.0;
		}
		
}

		

	
	


void Camera::keyCallback(GLFWwindow * window, int key, int code, int action, int mode)
{
	Camera* thisInstance = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	thisInstance->keyActions(key, action);
}

void Camera::cursorPositionCallback(GLFWwindow * window, double xpos, double ypos)
{
	Camera* thisInstance = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	thisInstance->mouseActions(xpos, ypos);

	thisInstance->cursorULast = xpos;
	thisInstance->cursorVLast = ypos;
}


