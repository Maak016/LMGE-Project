#include "inputSystem.h"

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight;
const float FOV = 45.0f;
const float mouseSensitivity = 0.001f;
double pitch = 0;
double yaw = 0;

const double moveSpeedNorm = 10.0f;
const double moveSpeedFast = 2.0 * moveSpeedNorm;
const float jumpHeight = 5.0f;
const float jumpStrength = 5.0f;

bool onGround = false;
bool jump = false;

constexpr float G = 9.81;
const float playerWeight = 70;

glm::vec3 lastPos = camPos;		//The position of the player in the last frame
float currentGroundLevel = -1.0f;

glm::vec3 inputMoveVector = glm::vec3(0.0f);
glm::vec3 currentVelocity = glm::vec3(0.0f);
void movementInputHandler(GLFWwindow* window) {
	camRight = glm::cross(camFront, camUp);
	glm::vec3 heading = -glm::cross(camRight, camUp);

	currentVelocity = camPos - lastPos;
	lastPos = camPos;

	//Player-Ground collision detection
	if (camPos.y > currentGroundLevel) onGround = false;
	else {
		onGround = true;
		camPos.y = currentGroundLevel;
		currentVelocity.y = 0;
	}

	//ground movement
	if (onGround) {
		glm::vec3 vector = glm::vec3(0.0f);
		vector += static_cast<float>(glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S)) * heading;
		vector += static_cast<float>(glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A)) * camRight;

		if (glm::length(vector) > 0.0f) inputMoveVector += glm::normalize(vector) * static_cast<float>(deltaTime * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? moveSpeedFast : moveSpeedNorm));
	}

	//ground counter movement
	if (onGround) {
		inputMoveVector.x -= currentVelocity.x * playerWeight * G * deltaTime * 5.0f;
		inputMoveVector.z -= currentVelocity.z * playerWeight * G * deltaTime * 5.0f;
	}

	//jumping
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) jump = true;

	if (jump && camPos.y < jumpHeight) currentVelocity.y += jumpStrength * 10.0f;
	else jump = false;

	//Gravity handling
	if(!onGround) currentVelocity.y -= G * playerWeight * deltaTime * 15.0f;

	currentVelocity += inputMoveVector;
	camPos += currentVelocity * (float)deltaTime;
}

glm::vec3 getMovementVector() { return currentVelocity; }
void addForce(glm::vec3 direction, float magnitude) {
	direction = glm::normalize(direction);
	direction *= magnitude * static_cast<float>(deltaTime);

	currentVelocity += direction;
}

double lastX = scrWidth / 2;
double lastY = scrHeight / 2;
bool firstMouseEnter = true;
void mouseMovementCallback(GLFWwindow* window, double xPos, double yPos) {
	if (firstMouseEnter) {
		lastX = xPos;
		lastY = yPos;

		firstMouseEnter = false;
	}

	double offsetX = xPos - lastX;
	double offsetY = yPos - lastY;

	lastX = xPos;
	lastY = yPos;

	offsetX *= mouseSensitivity;
	offsetY *= mouseSensitivity;

	yaw -= offsetX;
	pitch -= offsetY;

	camFront = glm::vec3(sin(yaw) * cos(pitch), sin(pitch), cos(yaw) * cos(pitch));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
