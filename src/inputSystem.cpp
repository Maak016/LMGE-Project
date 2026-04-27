#include "inputSystem.h"

const float FOV = 45.0f;
const float mouseSensitivity = 0.001f;
double pitch = 0;
double yaw = 0;

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

void capitalize(std::string& s) {
	for (char& c : s) {
		if (c >= 'a' && c <= 'z') c = char(c - ('a' - 'A'));
	}
}

GLFWwindow* input::runningWindow;
std::map<int, bool> pressedKeys;
std::map<int, bool> newPress;

void input::init(GLFWwindow* window) { runningWindow = window; }
bool input::getKey(std::string keyCode) {
	capitalize(keyCode);
	if (!keyCodes.contains(keyCode)) {
		std::cout << "ERROR: INPUT_SYSTEM: Key code input '" << keyCode << "' is invalid." << std::endl;
		engine::terminate();
		return false;
	}
	return pressedKeys.at(keyCodes.at(keyCode));
}
bool input::getNewPress(std::string keyCode) {
	capitalize(keyCode);
	if (!keyCodes.contains(keyCode)) {
		std::cout << "ERROR: INPUT_SYSTEM: Key code input '" << keyCode << "' is invalid." << std::endl;
		engine::terminate();
		return false;
	}
	return newPress.at(keyCodes.at(keyCode));
}

bool input::getMouse(std::string button) {
	capitalize(button);
	if (!mouseButtons.contains(button)) {
		std::cout << "ERROR: INPUT_SYSTEM: Key code input '" << button << "' is invalid." << std::endl;
		engine::terminate();
		return false;
	}
	return pressedKeys.at(mouseButtons.at(button));
}
bool input::getNewMousePress(std::string button) {
	capitalize(button);
	if (!mouseButtons.contains(button)) {
		std::cout << "ERROR: INPUT_SYSTEM: Key code input '" << button << "' is invalid." << std::endl;
		engine::terminate();
		return false;
	}
	return newPress.at(mouseButtons.at(button));
}

void input::activeScan() {
	for (const auto& key : keyCodes) {
		if (getKey(key.first) && getNewPress(key.first)) std::cout << "LOG: key pressed: " << key.first << " (NEW PRESS)" << std::endl;
		else if(getKey(key.first)) std::cout << "LOG: key pressed: " << key.first << std::endl;
	}
	for (const auto& button : mouseButtons) {
		if(getMouse(button.first) && getNewMousePress(button.first)) std::cout << "LOG: key pressed: " << button.first << " (NEW PRESS)" << std::endl;
		else if(getMouse(button.first)) std::cout << "LOG: key pressed: " << button.first << std::endl;
	}
}
void input::update() {
	for (const auto& key : keyCodes) {
		if (glfwGetKey(runningWindow, key.second) == GLFW_PRESS && !pressedKeys.at(key.second)) newPress[key.second] = true;
		else newPress[key.second] = false;

		if (glfwGetKey(runningWindow, key.second) == GLFW_PRESS) pressedKeys[key.second] = true;
		else pressedKeys[key.second] = false;
	}
	for (const auto& button : mouseButtons) {
		if (glfwGetMouseButton(runningWindow, button.second) == GLFW_PRESS && !pressedKeys.at(button.second)) newPress[button.second] = true;
		else newPress[button.second] = false;

		if (glfwGetMouseButton(runningWindow, button.second) == GLFW_PRESS) pressedKeys[button.second] = true;
		else pressedKeys[button.second] = false;
	}
	//std::cout << newPress.size() << '\n' << pressedKeys.size() << std::endl;
}