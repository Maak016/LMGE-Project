#include "gameObj.h"
#include "shapes.h"

std::vector<gameObject*> allObjects;

void gameObject::init(model objectModel, shader renderShader, void(*init)(void), void(*update)(void), std::vector<std::vector<glm::vec3>> col) {
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

void gameObject::update() {
	if (updateFunc != nullptr)
	updateFunc();

	for (int i = 0; i < instances.size(); i++) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);

		//rotates the object with i(th) rotation vector
		modelMatrix = glm::rotate(modelMatrix, instances[i].rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, instances[i].rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, instances[i].rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

		modelMatrix = glm::translate(modelMatrix, instances[i].pos);		//translates the object with the i(th) vector

		objectModel.draw(renderShader, modelMatrix);

		if (collidable) {
			if (collision(allObjects, instances[i].collidees, i) || instances[i].colliding) {
				std::cout << "Collision = TRUE" << std::endl;

				if (onCollision != nullptr) onCollision();
			}
		}
	}
}
void gameObject::postFrameCleanup() {
	for (int i = 0; i < instances.size(); i++) {
		instances[i].colliding = false;
		instances[i].collidees.clear();
	}
}

void gameObject::runtimeInit() { if(initFunc != nullptr) initFunc(); }

void gameObject::instantiate(glm::vec3 pos, glm::vec3 rot) { instances.push_back({this->objectID, pos, rot, false, false, {}}); }
void gameObject::instantiate(float x, float y, float z, float rotX, float rotY, float rotZ) {
	glm::vec3 pos = glm::vec3(x, y, z);
	glm::vec3 rot = glm::vec3(rotX, rotY, rotZ);
	instances.push_back({this->objectID, pos, rot, false, false, {}});
}

std::vector<std::vector<glm::vec3>> gameObject::hitbox() {
	return hitboxRegion;
}

//sets the collision state of an instace of another object, usually to "inform" the collidee that it has collided with the current object
void gameObject::collisionState(bool state, unsigned int collideeIndex, instance* collider) {
	if(state)
	this->instances[collideeIndex].collidees.push_back(collider);
	this->instances[collideeIndex].colliding = state;
}

bool gameObject::collision(std::vector<gameObject*>& all, std::vector<instance*>& outputObj, unsigned int colliderIndex) {
	glm::mat4 pos = glm::mat4(1.0f);

	pos = glm::rotate(pos, glm::radians(instances[colliderIndex].rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
	pos = glm::rotate(pos, glm::radians(instances[colliderIndex].rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	pos = glm::rotate(pos, glm::radians(instances[colliderIndex].rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	pos = glm::translate(pos, instances[colliderIndex].pos);

	for (int i = 0; i < all.size(); i++) {
		if (all[i]->getObjectID() == this->objectID) continue;

		for (int j = 0; j < all[i]->instances.size(); j++) {
			//check for collision: returns true if the coordinates of the two objects have a common area.
			bool collision = false;

			//iterate through each point forming the hitbox of the (possible) collidee and check if it is inside the current instance
			std::vector<glm::vec3> points;
			for (int k = 0; k < all[i]->hitbox().size(); k++) {
				for (glm::vec3 &vertex : all[i]->hitbox()[k]) points.push_back(vertex);
			}

			//if one of the current iterating instances is found to have a common area with the subject, the collision state (colliding) of both objects will be set to true
			std::vector<segment*> collider = genSegments(this->hitboxRegion);
			for (int k = 0; k < points.size(); k++) {
				if (pointInPolygon(points.at(k), pos, collider)) {
					all[i]->collisionState(true, j, &this->instances[colliderIndex]);

					this->instances[colliderIndex].colliding = true;
					this->instances[colliderIndex].collidees.push_back(&all[i]->instances[j]);

					collision = true;
					break;
				}
			}

			//move on to the next iteration if no collision between the two instances detected
			if (!collision) continue;
			outputObj.push_back(&all[i]->instances[j]);
		}
	}

	if (outputObj.size() > 0) return true;
	return false;
}

void gameObject::enableCollision(bool val) {
	collidable = val;
}