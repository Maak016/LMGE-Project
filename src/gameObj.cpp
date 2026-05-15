#include "gameObj.h"
#include "maths.h"

std::vector<gameObject*> allObjects;

void gameObject::init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), std::vector<std::vector<glm::vec3>> col) {
#ifndef LEGACY_COLLISION
	std::cout << "WARNING: Legacy gameObject initialization function used while Legacy Collision system is disabled. The hitbox Region param will not have any effect" << std::endl;
#endif

	this->objectModel = objectModel;
	this->initFunc = init;
	this->updateFunc = update;
	this->renderShader = renderShader;

	if (!col.empty()) {
		hitboxRegion = col;
		collidable = true;
	}
	else collidable = false;
	
	allObjects.push_back(this);
	this->objectID = allObjects.size();
}

void gameObject::init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), model* colliderModel) {
	this->objectModel = objectModel;
	this->initFunc = init;
	this->updateFunc = update;
	this->renderShader = renderShader;

	if (colliderModel != nullptr) {
		this->collider = *colliderModel;
		colliderModelDefined = true;
	}
	collidable = true;

	allObjects.push_back(this);
	this->objectID = allObjects.size();
}

void gameObject::update() {
	if (collidable && isTrigger) {
		std::cout << "ERROR: Game Object cannot both be trigger and collidable." << std::endl;
		engine::terminate();
	}

	if (updateFunc != nullptr) updateFunc();

#ifdef DISABLE_INSTANCING
	for (int i = 0; i < instances.size(); i++) {
		if (instances[i].destroyed) continue;

		glm::mat4 modelMatrix = glm::mat4(1.0f);

		modelMatrix = glm::translate(modelMatrix, instances[i].pos);		//translates the object with the i(th) vector

		//rotates the object with i(th) rotation vector
		modelMatrix = glm::rotate(modelMatrix, glm::radians(instances[i].rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(instances[i].rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(instances[i].rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

		objectModel.draw(renderShader, modelMatrix);

		if (collidable) {
			if (collision(allObjects, instances[i].collidees, i) || instances[i].colliding) {
				std::cout << "Collision = TRUE" << std::endl;

				if (onCollision != nullptr) onCollision();
			}
		}
	}
#else
	instanceModel = getInstanceModel();
	bindInstanceModel();

	objectModel.drawInstanced(renderShader, instances.size());

	for (int i = 0; i < instances.size(); i++) {
		if (instances[i].destroyed) continue;

		if (collidable) {
			if (collision(allObjects, instances[i].collidees, i) || instances[i].colliding) {
				if (onCollision != nullptr) onCollision();
			}
		}
		else if (isTrigger) {
			if (collision(allObjects, instances[i].collidees, i) || instances[i].colliding) {
				if (onCollision != nullptr) onCollision();
			}
		}

		if (physicsModelEnabled && physicsAttribLoaded) simObjectPhysics(i);
		if (stationary) instances[i].pos = instances[i].lastPos;
	}

#endif
}
void gameObject::postFrameCleanup() {
	for (int i = 0; i < instances.size(); i++) {
		instances[i].colliding = false;
		instances[i].collidees.clear();
	}
	this->checkedInstances.clear();
}

void gameObject::runtimeInit() { if(initFunc != nullptr) initFunc(); }

void gameObject::instantiate(glm::vec3 pos, glm::vec3 rot) { 
	instances.push_back({this->objectID, pos, rot, false, false, {}, pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) }); 
	if (stationary) this->instances[instances.size() - 1].lastPos = pos;
}
void gameObject::instantiate(float x, float y, float z, float rotX, float rotY, float rotZ) {
	glm::vec3 pos = glm::vec3(x, y, z);
	glm::vec3 rot = glm::vec3(rotX, rotY, rotZ);
	instances.push_back({this->objectID, pos, rot, false, false, {}, pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)});
	if (stationary) this->instances[instances.size() - 1].lastPos = pos;
}

std::vector<std::vector<glm::vec3>> gameObject::hitbox() { return hitboxRegion; }

void gameObject::setAxisAlignedHitbox(bool state) { this->axisAlignedHitbox = state; }
bool gameObject::axisAlignedHitboxState() { return this->axisAlignedHitbox; }

//sets the collision state of an instace of another object, usually to "inform" the collidee that it has collided with the current object
void gameObject::collisionState(bool state, unsigned int collideeIndex, instance* collider) {
	if(state)
	this->instances[collideeIndex].collidees.push_back(collider);
	this->instances[collideeIndex].colliding = state;
}

bool gameObject::collision(std::vector<gameObject*>& all, std::vector<instance*>& outputObj, unsigned int colliderIndex) {
	for (int i = 0; i < all.size(); i++) {
		for (int j = 0; j < all[i]->instances.size(); j++) {
			bool validCheck = true;

			//optimization: check if the current pair of instances have been checked for collision in the past in the current frame, thus reducing total number of checks
			if (all[i]->getObjectID() == this->objectID && j == colliderIndex) validCheck = false;
			
			for (const std::vector<unsigned int>& instance : checkedInstances)
				if (instance[0] == all[i]->getObjectID() && instance[1] == j) validCheck = false;
			for (const std::vector<unsigned int>& instance : all[i]->checkedInstances)
				if (instance[0] == this->objectID && instance[1] == colliderIndex) validCheck = false;

			if (!validCheck) continue;
			//check passed: add the current pair to the history of checked pairs in the current frame
			this->checkedInstances.push_back({all[i]->getObjectID(), (unsigned int)j});
			all[i]->checkedInstances.push_back({ this->objectID, colliderIndex });

			//check for collision: returns true if the coordinates of the two objects have a common area.
			bool collision = false;

#ifndef LEGACY_COLLISION
			if (separatingAxisTest(this, all[i], colliderIndex, j)) {
				all[i]->collisionState(true, j, &this->instances[colliderIndex]);

				this->instances[colliderIndex].colliding = true;
				this->instances[colliderIndex].collidees.push_back(&all[i]->instances[j]);

				collision = true;
			}
#else

			//iterate through each point forming the hitbox of the (possible) collidee and check if it is inside the current instance
				//transform the hitbox region (in local coords) to the collidees coords first
			glm::mat4 collideeModel = all[i]->getPosMatrix(j);
			std::vector<std::vector<glm::vec3>> collideeHitbox = all[i]->hitbox();
			std::vector<glm::vec3> points;
			
			for (std::vector<glm::vec3>& shape : collideeHitbox) {
				for (glm::vec3& point : shape) {
					points.push_back(glm::vec3(collideeModel * glm::vec4(point, 1.0f)));
				}
			}

			//if one of the current iterating instances is found to have a common area with the subject, the collision state (colliding) of both objects will be set to true
				//transform the hitbox region of the collider
			glm::mat4 colliderModel = getPosMatrix(colliderIndex);
			std::vector<std::vector<glm::vec3>> transformedHitbox;

			for (int k = 0; k < this->hitboxRegion.size(); k++) {
				if (transformedHitbox.size() < k + 1) transformedHitbox.push_back({});

				std::vector<glm::vec3> current = this->hitboxRegion.at(k);

				for (glm::vec3& coord : current) {
					transformedHitbox[k].push_back(glm::vec3(colliderModel * glm::vec4(coord, 1.0f)));
				}
			}

			std::vector<segment*> collider = genSegments(transformedHitbox);
			for (int k = 0; k < points.size(); k++) {
				if (pointInPolygon(points.at(k), colliderModel, collider)) {
					all[i]->collisionState(true, j, &this->instances[colliderIndex]);

					this->instances[colliderIndex].colliding = true;
					this->instances[colliderIndex].collidees.push_back(&all[i]->instances[j]);

					collision = true;
					break;
				}
			}

#endif

			//move on to the next iteration if no collision between the two instances detected
			if (!collision) continue;
			outputObj.push_back(&all[i]->instances[j]);

#ifdef DEBUG_WORLD_PHYSICS
			std::cout << "LOG: Object with ID " << this->objectID << " colliding with: " << all[i]->getObjectID() << " index " << j << std::endl;
#endif
		}
	}

	if (outputObj.size() > 0) return true;
	return false;
}

void gameObject::enableCollision(bool val) {
	collidable = val;
	if (val) isTrigger = false;
}
void gameObject::makeTrigger(bool val) {
	isTrigger = val;
	if (val) collidable = false;
}

glm::vec3 gameObject::getPos(const unsigned int index) {
	return instances[index].pos;
}
glm::vec3 gameObject::getRot(const unsigned int index) {
	return instances[index].rot;
}

glm::mat4 gameObject::getPosMatrix(const unsigned int index) {
	glm::mat4 result = glm::mat4(1.0f);
	
	result = glm::translate(result, instances[index].pos);

	result = glm::rotate(result, glm::radians(instances[index].rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result = glm::rotate(result, glm::radians(instances[index].rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result = glm::rotate(result, glm::radians(instances[index].rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	return result;
}

shader gameObject::getRenderShader() { return renderShader; }

model gameObject::getModel() {
	if (colliderModelDefined) return collider;
	return objectModel;
}

std::vector<glm::mat4> gameObject::getInstanceModel() {
	std::vector<glm::mat4> result;
	for (int i = 0; i < instances.size(); i++) {
		if (instances[i].destroyed) continue;
		result.push_back(getPosMatrix(i));
	}

	return result;
}

void gameObject::bindInstanceModel() {
	std::vector<Mesh> meshes = this->objectModel.getModelMesh();

	//transfer data from vector to normal array
	glm::mat4* matrices = instanceModel.data();

	for (int i = 0; i < meshes.size(); i++) {
		unsigned int currentVAO = meshes[i].getVertexArray();

		glBindVertexArray(currentVAO);

		unsigned int instanceVBO;
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, instanceModel.size() * sizeof(glm::mat4), matrices, GL_STATIC_DRAW);

		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)sizeof(glm::vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		for (int j = 3; j <= 6; j++) {
			glEnableVertexAttribArray(j);
			glVertexAttribDivisor(j, 1);
		}

		glBindVertexArray(0);
	}
}

void gameObject::translate(const unsigned int instanceIndex, glm::vec3 dir, float mag) {
	if (instanceIndex < 0 || instanceIndex >= this->instances.size()) {
		std::cout << std::endl << "ERROR: Specified index for object translation is OUT OF BOUND." << std::endl;
		std::cout << "INFO: Specified Index: " << instanceIndex << '\n' << "; Maximum index: " << this->instances.size() - 1 << std::endl;
		engine::terminate();

		return;
	}
	this->instances[instanceIndex].pos += dir * static_cast<float>(mag);
}
void gameObject::translateRotation(const unsigned int instanceIndex, glm::vec3 axis, float mag) {
	if (instanceIndex < 0 || instanceIndex >= this->instances.size()) {
		std::cout << std::endl << "ERROR: Specified index for object rotation is OUT OF BOUND." << std::endl;
		std::cout << "INFO: Specified Index: " << instanceIndex << '\n' << "; Maximum index: " << this->instances.size() - 1 << std::endl;
		engine::terminate();

		return;
	}

	this->instances[instanceIndex].rot += axis * static_cast<float>(mag);

	if (this->instances[instanceIndex].rot.x >= 360.0f) this->instances[instanceIndex].rot.x -= 360 * floor(this->instances[instanceIndex].rot.x / 360.0f);
	if (this->instances[instanceIndex].rot.y >= 360.0f) this->instances[instanceIndex].rot.y -= 360 * floor(this->instances[instanceIndex].rot.y / 360.0f);
	if (this->instances[instanceIndex].rot.z >= 360.0f) this->instances[instanceIndex].rot.z -= 360 * floor(this->instances[instanceIndex].rot.z / 360.0f);
}
void gameObject::rotate(const unsigned int instanceIndex, glm::vec3 rotation) {
	if (instanceIndex < 0 || instanceIndex >= this->instances.size()) {
		std::cout << std::endl << "ERROR: Specified index for object translation is OUT OF BOUND." << std::endl;
		std::cout << "INFO: Specified Index: " << instanceIndex << '\n' << "; Maximum index: " << this->instances.size() << std::endl;
		engine::terminate();

		return;
	}
	this->instances[instanceIndex].rot = rotation;
}

bool gameObject::trigger() { return this->isTrigger; }



void gameObject::freeInstanceMemory(const unsigned int instanceIndex) {
	if (instanceIndex >= this->instances.size()) {
		std::cout << "ERROR: instance index specified in freeInstanceMemory() is OUT OF BOUND." << std::endl;
		engine::terminate();
	}

	//moves the instance to the end of the array
	for (int i = instanceIndex; i < this->instances.size() - 1; i++) {
		instance& temp = instances.at(i + 1);
		instances.at(i + 1) = instances.at(i);
		instances.at(i) = temp;
	}

	instances.pop_back();	//delete the instance, having moved it to the end of the array
}
void gameObject::destroyInstance(const unsigned int instanceIndex) { this->instances[instanceIndex].destroyed = true; }
void gameObject::reinstate(const unsigned int instanceIndex) { this->instances[instanceIndex].destroyed = false; }

/*
* weight: the weight in kilograms of the object. this impacts the effect of forces acting on the object
* softness: 0.0f means hardest, 1.0f is softest
*/
void gameObject::initializePhysicsModel(float weight, float softness, float bounciness) {
	this->objectWeight = weight;
	this->softness = softness;
	this->bounciness = bounciness;

	this->physicsAttribLoaded = true;
	this->physicsModelEnabled = true;
}
void gameObject::setPhysicsModelStatus(bool state) {
	if (state && !physicsAttribLoaded) {
		std::cout << "ERROR: Object's physics attributes not loaded. Please use gameObject::initializePhysicsModel() first." << std::endl;
		engine::terminate();
		return;
	}
	physicsModelEnabled = state;
}
void gameObject::setStationaryState(bool state) { this->stationary = state; }

bool gameObject::physicsModelLoadStatus() { return this->physicsAttribLoaded; }
float gameObject::getWeight() { return this->objectWeight; }
float gameObject::getSoftness() { return this->softness; }
float gameObject::getBounciness() { return this->bounciness; }
bool gameObject::getStationaryState() { return this->stationary; }

float gameObject::getSpeed(const unsigned int instanceIndex) {
	return glm::length(instances[instanceIndex].currentVelocity) / deltaTime;
}
float gameObject::getSpeed(const unsigned int instanceIndex, glm::vec3 dir) {
	if (glm::length(instances[instanceIndex].currentVelocity) == 0.0f) return 0.0f;

	glm::vec3 v = glm::normalize(instances[instanceIndex].currentVelocity);
	glm::vec3 direction = glm::normalize(dir);

	float cosTheta = glm::dot(v, direction) / (glm::length(v) * glm::length(direction));

	return (glm::length(instances[instanceIndex].currentVelocity) * cosTheta / deltaTime);
}

void gameObject::addForce(const unsigned int instanceIndex, glm::vec3 dir, float mag) {
	if (!this->physicsModelEnabled || !this->physicsAttribLoaded)
		std::cout << "WARNING: Object's physics model is not enabled or does not have initialized data. Input force will NOT have any effect." << std::endl;
	
	this->instances[instanceIndex].inputMoveVector += (dir * mag * 0.05f) / (this->objectWeight * 0.5f);
}

bool grav = false;
void gameObject::simObjectPhysics(const unsigned int instanceIndex) {
	instance* currentInstance = &this->instances[instanceIndex];

	if (this->stationary) return;
	//simulate gravity
#ifndef DISABLE_GRAVITY
	if (grav) this->addForce(instanceIndex, glm::vec3(0.0f, -1.0f, 0.0f), physicsConstants::G);
#endif

	currentInstance->currentVelocity += currentInstance->inputMoveVector;
	currentInstance->pos += (currentInstance->currentVelocity) * static_cast<float>(deltaTime);
	currentInstance->inputMoveVector = glm::vec3(0.0f);
}

#include "inputSystem.h"
gameObject engine::player;
//namespace engine

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight;

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

void engine::defaultPlayerControl() {
	camRight = glm::cross(camFront, camUp);
	glm::vec3 heading = -glm::cross(camRight, camUp);

	//Player ground contact detection
#if 0

	camRight = glm::cross(camFront, camUp);
	glm::vec3 heading = -glm::cross(camRight, camUp);

	//Player-Ground collision detection
	if (player.instances[0].pos.y > currentGroundLevel) onGround = false;
	else {
		onGround = true;
		player.instances[0].pos.y = currentGroundLevel;
		player.instances[0].currentVelocity.y = 0;
	}

	//ground movement
	if (onGround) {
		glm::vec3 vector = glm::vec3(0.0f);
		vector += static_cast<float>(input::getKey("w") - input::getKey("s")) * heading;
		vector += static_cast<float>(input::getKey("d") - input::getKey("a")) * camRight;

		//if (glm::length(vector) > 0.0f) player.instances[0].inputMoveVector += glm::normalize(vector) * static_cast<float>(deltaTime * (input::getKey("left_shift") ? moveSpeedFast : moveSpeedNorm));
		if(glm::length(vector) > 0.0f)
		player.addForce(0, glm::normalize(vector), static_cast<float>(deltaTime * (input::getKey("left_shift") ? moveSpeedFast : moveSpeedNorm)));
	}

	//ground counter movement
	if (onGround) {
		//player.instances[0].inputMoveVector.x -= player.instances[0].currentVelocity.x * player.getWeight() * G * deltaTime * 10.0f;
		//player.instances[0].inputMoveVector.z -= player.instances[0].currentVelocity.z * player.getWeight() * G * deltaTime * 10.0f;
		glm::vec3 xz = glm::vec3(player.instances[0].currentVelocity.x, 0.0f, player.instances[0].currentVelocity.z);

		player.addForce(0, -glm::normalize(xz), glm::length(xz) * player.getWeight() * G * 10.0f);
	}

	//jumping
	if (input::getKey("space") && onGround) jump = true;

	if (jump && player.instances[0].pos.y < jumpHeight) player.instances[0].currentVelocity.y += jumpStrength * 10.0f;
	else jump = false;

	//Gravity handling
	if (!onGround) player.instances[0].currentVelocity.y -= G * player.getWeight() * deltaTime * 15.0f;

	//player.instances[0].currentVelocity += player.instances[0].inputMoveVector;
	//player.instances[0].pos += player.instances[0].currentVelocity * (float)deltaTime;
	//player.instances[0].inputMoveVector += player.instances[0].currentVelocity + player.instances[0].inputMoveVector;

#ifdef DEBUG_PLAYER_PHYSICS
	std::cout << "-------Player physics properties-------\n";
	std::cout << "LOG: Current player position: " << '(' << camPos.x << ',' << camPos.y << ',' << camPos.z << ')' << '\n';
	std::cout << "LOG: Current player move velocity: " << '(' << currentVelocity.x << ',' << currentVelocity.y << ',' << currentVelocity.z << ')' << '\n';
	std::cout << "LOG: Player jumping/in air status: " << !onGround << '\n';

	std::cout << std::endl;
#endif
	camPos = player.instances[0].pos;

#endif
}


void engine::setupPlayerCollider(shader& renderShader) {
	model hoomanModel("assets/defaultAssets/playerModel/model.obj");
	player.init(hoomanModel, renderShader, nullptr, defaultPlayerControl, nullptr);
	player.initializePhysicsModel(70.0f, 1.0f, 1.0f);

	player.instantiate(camPos, glm::vec3(0.0f, 0.0f, 0.0f));
}
void engine::setupPlayerCollider() {
	model hoomanModel("assets/defaultAssets/playerModel/model.obj");
	player.init(hoomanModel, *getRenderShader(), nullptr, defaultPlayerControl, nullptr);
	player.initializePhysicsModel(70.0f, 1.0f, 1.0f);

	player.instantiate(camPos, glm::vec3(0.0f, 0.0f, 0.0f));
}