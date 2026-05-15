#pragma once
#include "fundamental.h"

extern bool grav;
namespace engine {
	bool engineTerminated();
	void terminate();
	//void setupPlayerCollider();
	enum class defaultID{player = 1, terrain = 2};
}
namespace physicsConstants {
	const float G = 9.81;
}