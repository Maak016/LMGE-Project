#pragma once
#include "fundamental.h"

namespace engine {
	bool engineTerminated();
	void terminate();
	//void setupPlayerCollider();
}
namespace physicsConstants {
	const float G = 9.81;
}