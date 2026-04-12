#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <sstream>
#include <fstream>

#include <algorithm>
#include <cmath>
#include <ctime>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

//#define DEBUG_WORLD_PHYSICS
//#define DEBUG_PLAYER_PHYSICS
//#define DEBUG_RENDERING
//#define DEBUG_INPUT_SYSTEM

//#define LEGACY_INPUT_SYSTEM
//#define DISABLE_LIGHTING
//#define DISABLE_INSTANCING
//#define LEGACY_COLLISION
//#define COLLSION_DETECTION_ONLY
#define DISABLE_GRAVITY

extern double deltaTime;

namespace engine {
	void terminate();
}
namespace physicsConstants {
	const float G = 9.81;
}