#include "inputSystem.h"

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight;
const float FOV = 45.0f;
const float mouseSensitivity = 0.3f;
double pitch = 0;
double yaw = 0;

const double moveSpeedNorm = 0.1;
const double moveSpeedFast = 1.5 * moveSpeedNorm;
const float jumpStrength = 10.0f;
bool jumping = false;

constexpr float G = 9.81;
const float playerWeight = 70;

glm::vec3 moveVector = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 lastPos = glm::vec3(camPos.x, 0.0f, camPos.z);
void inputHandler(GLFWwindow* window) {
	camRight = glm::normalize(glm::cross(camFront, camUp));
	
	glm::vec3 currentDir;

	currentDir = lastPos == camPos ? glm::vec3(0.0f, 0.0f, 0.0f) : glm::normalize(camPos - lastPos);

#ifdef DEBUGMODE
	std::cout << std::endl << "--Player movement properties-- \n";
	std::cout << "current movement direction: " << currentDir.x << ',' << currentDir.y << ',' << currentDir.z << std::endl;
	std::cout << "camera (player) pos: " << camPos.x << ',' << camPos.y << ',' << camPos.z << std::endl;
#endif

	glm::vec3 counterMovement = -currentDir * G * 0.01f;
	moveVector += counterMovement;

	lastPos = glm::vec3(camPos.x, 0.0f, camPos.z);
	if (camPos.y > -1.0f) {
		moveVector += glm::vec3(0.0f, -1.0f, 0.0f) * 0.00001f * G * (float)deltaTime;
		jumping = true;
	}
	else {
		moveVector.y = 0.0f;
		jumping = false;
	}

	if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_A)) {
		//The forward vector relative to the camera is the cross product of the right and up vector
		glm::vec3 fwd = glm::normalize(glm::cross(camRight, camUp));

		glm::vec3 groundMovement = glm::vec3(0.0f, 0.0f, 0.0f);

		int movement = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
		groundMovement -= fwd * static_cast<float>(movement);

		int movementLat = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);
		groundMovement += camRight * static_cast<float>(movementLat);

		groundMovement = glm::normalize(groundMovement);

		moveVector += groundMovement * static_cast<float>(moveSpeedNorm * deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) && !jumping) {
		moveVector += camUp * static_cast<float>(jumpStrength * deltaTime);
	}

	camPos += moveVector;
}
glm::vec3 getMovementVector() { return moveVector; }
void addForce(glm::vec3 direction, float magnitude) {
	direction = glm::normalize(direction);
	direction *= magnitude * static_cast<float>(deltaTime);

	moveVector += direction;
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

	offsetX *= mouseSensitivity * deltaTime;
	offsetY *= mouseSensitivity * deltaTime;

	yaw -= offsetX;
	pitch -= offsetY;

	camFront = glm::vec3(sin(yaw) * cos(pitch), sin(pitch), cos(yaw) * cos(pitch));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
