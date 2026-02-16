#pragma once

#include "fundamental.h"

extern const int scrWidth;
extern const int scrHeight;

extern glm::vec3 camPos;
extern glm::vec3 camFront;
extern double pitch, yaw;

extern const double moveSpeedNorm;
extern const double moveSpeedFast;

extern const float FOV;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseMovementCallback(GLFWwindow* window, double xPos, double yPos);
void inputHandler(GLFWwindow* window);
glm::vec3 getMovementVector();
void addForce(glm::vec3 direction, float magnitude);