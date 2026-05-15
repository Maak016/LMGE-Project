#pragma once
#include "fundamental.h"
#include "modelLoading.h"

class gameObject {
private:
	unsigned int objectID;

	model objectModel;
	model collider;
	bool colliderModelDefined = false;

	shader renderShader;
	std::vector<std::vector<glm::vec3>> hitboxRegion;
	bool displayHitbox = false;
	bool axisAlignedHitbox = false;

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

		glm::vec3 lastPos;
		glm::vec3 currentVelocity;
		glm::vec3 inputMoveVector;
	};
	std::vector<glm::mat4> instanceModel;

	bool collidable;
	bool isTrigger;
	bool stationary = false;

	bool physicsModelEnabled = false;
	bool physicsAttribLoaded = false;

	float objectWeight;
	float softness;
	float bounciness;

	bool collision(std::vector<gameObject*>& all, std::vector<instance*>& outputObj, unsigned int colliderIndex);
	void simObjectPhysics(const unsigned int instanceIndex);
	std::vector<glm::mat4> getInstanceModel();
	void bindInstanceModel();

	std::vector<std::vector<unsigned int>> checkedInstances; //form: {{i, j}} with i being the gameObject ID and the j being the instance index
public:
	std::vector<instance> instances;

	unsigned int getObjectID() { return objectID; }

	void init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), std::vector<std::vector<glm::vec3>> col);
	void init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), model* colliderModel);

	void instantiate(glm::vec3 pos, glm::vec3 rot);
	void instantiate(float x, float y, float z, float rotX, float rotY, float rotZ);
	void update();
	void runtimeInit();
	void postFrameCleanup();

	void collisionState(bool state, unsigned int collideeIndex, instance* collider);
	void enableCollision(bool val);
	void makeTrigger(bool val);
	std::vector<std::vector<glm::vec3>> hitbox(); 
	void setAxisAlignedHitbox(bool state);
	bool axisAlignedHitboxState();

	glm::vec3 getPos(const unsigned int index);
	glm::vec3 getRot(const unsigned int index);
	glm::mat4 getPosMatrix(const unsigned int index);
	shader getRenderShader();

	//getting the model used for collision of the object 
	model getModel();

	void translate(const unsigned int instanceIndex, glm::vec3 dir, float mag);
	void translateRotation(const unsigned int instanceIndex, glm::vec3 axis, float mag);
	void rotate(const unsigned int instanceIndex, glm::vec3 rotation);

	bool trigger();

	void initializePhysicsModel(float weight, float softness, float bounciness);
	bool physicsModelLoadStatus();
	void setPhysicsModelStatus(bool state);
	void setStationaryState(bool state);

	float getWeight();
	float getSoftness();
	float getBounciness();
	bool getStationaryState();
	
	float getSpeed(const unsigned int instanceIndex);
	float getSpeed(const unsigned int instanceIndex, glm::vec3 dir);

	void destroyInstance(const unsigned int instanceIndex);
	void reinstate(const unsigned int instanceIndex);
	void freeInstanceMemory(const unsigned int instanceIndex);

	void addForce(const unsigned int instanceIndex, glm::vec3 dir, float mag);
};

extern gameObject player;
extern std::vector<gameObject*> allObjects;

namespace engine {
	extern gameObject player;
	void setupPlayerCollider();
	void setupPlayerCollider(shader& renderShader);

	void defaultPlayerControl();
}