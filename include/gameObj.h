#pragma once
#include "fundamental.h"
#include "modelLoading.h"

class gameObject {
private:
	unsigned int objectID;

	model objectModel;
	shader renderShader;
	std::vector<std::vector<glm::vec3>> hitboxRegion;
	bool displayHitbox = false;

	void(*initFunc)(void) = nullptr;
	void(*updateFunc)(void) = nullptr;
	void(*onCollision)(void) = nullptr;

	struct instance {
		unsigned int parentID;

		glm::vec3 pos;
		glm::vec3 rot;
		bool destroyed = false;
		bool colliding = false;
		std::vector<instance*> collidees;
	};
	std::vector<glm::mat4> instanceModel;

	bool collidable;
	bool collision(std::vector<gameObject*>& all, std::vector<instance*>& outputObj, unsigned int colliderIndex);

	std::vector<glm::mat4> getInstanceModel();
	void bindInstanceModel();
public:
	std::vector<instance> instances;

	unsigned int getObjectID() { return objectID; }

	void init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), std::vector<std::vector<glm::vec3>> col);
	
	void instantiate(glm::vec3 pos, glm::vec3 rot);
	void instantiate(float x, float y, float z, float rotX, float rotY, float rotZ);
	void update();
	void runtimeInit();
	void postFrameCleanup();

	void collisionState(bool state, unsigned int collideeIndex, instance* collider);
	void enableCollision(bool val);
	std::vector<std::vector<glm::vec3>> hitbox(); 

	glm::vec3 getPos(const unsigned int index);
	glm::vec3 getRot(const unsigned int index);
	glm::mat4 getPosMatrix(const unsigned int index);
	shader getRenderShader();
};

extern std::vector<gameObject*> allObjects;