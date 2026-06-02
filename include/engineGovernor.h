#pragma once
#include "fundamental.h"

extern bool grav;
namespace engine {
	bool engineTerminated();
	void terminate();
	//void setupPlayerCollider();
	enum class defaultID{player = 1, terrain = 2};
	void declareGameObjectCreation(void(*func)(void));

	void loadScene();

	namespace fileSystem {
		void createObjectScript(const unsigned int scene, const std::string name);
	}
	namespace exception {
		void throwFatalError(const std::string msg);
	}
}
namespace physicsConstants {
	const float G = 9.81;
}