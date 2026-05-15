#include "fundamental.h"
#include "shader.h"
#include "inputSystem.h"
#include "gameObj.h"

double deltaTime = 0;
double currentTime = 0;

const int scrWidth = 1280;
const int scrHeight = 720;

bool mouseInput = false;

//just for testing
gameObject yes;
gameObject yes2;
gameObject yesGround;
void updateYes() {
	if (input::getNewPress("g")) yes.addForce(0, yes2.getPos(0) - yes.getPos(0), 15.5f);
	if (input::getNewPress("b")) yes.addForce(0, -(yes2.getPos(0) - yes.getPos(0)), 15.5f);

	//if (glm::length(yes.instances[0].inputMoveVector) > 0)
	//std::cout << yes.instances[0].inputMoveVector.x << ' ' << yes.instances[0].inputMoveVector.y << ' ' << yes.instances[0].inputMoveVector.z << std::endl;
	//std::cout << yes.instances[0].currentVelocity.y << std::endl;
}
void updateNo() {
	//yesGround.instances[0].pos = glm::vec3(4.0f, -4.0f, -3.0f);
	//yes2.instances[0].pos = glm::vec3(4.0f, 1.0f, 3.0f);
}
void setupYes() {
	model yesM("assets/scene2/box/box.obj");
	yes.init(yesM, *engine::getRenderShader(), nullptr, updateYes, nullptr);

	yes.initializePhysicsModel(50.0f, 0.0f, 1.0f);

	yes.instantiate(4.0f, 1.0f, -3.0f, 0.0f, 0.0f, 0.0f);
}
void setupNo() {
	model yesM("assets/scene2/box/box.obj");
	yesGround.init(yesM, *engine::getRenderShader(), nullptr, updateNo, nullptr);

	yesGround.setStationaryState(true);
	yesGround.initializePhysicsModel(10.0f, 0.0f, 0.0f);

	yesGround.instantiate(4.1f, -4.0f, -3.1f, 0.0f, 0.0f, 0.0f);
}
void setupYes2() {
	model yesM("assets/scene2/box/box.obj");
	yes2.init(yesM, *engine::getRenderShader(), nullptr, nullptr, nullptr);

	yes2.setStationaryState(1);
	yes2.initializePhysicsModel(10.0f, 0.0f, 5.0f);

	yes2.instantiate(4.0f, 1.0f, 3.0f, 0.0f, 0.0f, 0.0f);
}
//

int main() {
	if (glfwInit() != GLFW_TRUE) std::cout << "ERROR: GLFW initialization failed." << std::endl;
	else std::cout << "SUCCESSFUL: GLFW initialization." << std::endl;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* mainWindow = glfwCreateWindow(scrWidth, scrHeight, "yes", nullptr, nullptr);
	glfwMakeContextCurrent(mainWindow);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "ERROR: OpenGL Loader failed." << std::endl;
		glfwTerminate();
		return -1;
	}
	else std::cout << "SUCCESSFUL: OpenGL Loader." << std::endl;
	std::cout << std::endl;

	unsigned int matricesBlock;
	glGenBuffers(1, &matricesBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesBlock);
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_STATIC_DRAW);	//2 * matrix4 = 2 * (4 * 4N) = 32N = 128bytes
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

#ifdef DISABLE_LIGHTING
#ifdef DISABLE_INSTANCING
	shader mainShader("shaders/noInstancing.lmv", "shaders/main.lmf");
#else
	shader mainShader("shaders/noInstancing.lmv", "shaders/noLighting.lmf");
#endif

#else

#ifdef DISABLE_INSTANCING
	shader mainShader("shaders/main.lmv", "shaders/noLighting.lmf");
#else
	shader mainShader("shaders/main.lmv", "shaders/main.lmf");
#endif

#endif
	engine::setRenderShader(&mainShader);
	engine::setupPlayerCollider();
	
	skybox normSkybox;
	normSkybox.init("assets/defaultAssets/skybox", ".jpg");

	setupYes();
	setupNo();
	setupYes2();

	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective(FOV, (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);

	//runs the one time initialization code of each gameObject object.
	for (int i = 0; i < allObjects.size(); i++) allObjects[i]->runtimeInit();

	glm::vec3 dirLightV = glm::normalize(glm::vec3(1.0f, 1.5f, 0.5f));
	lightSource dir(glm::vec3(0.0f, 0.0f, 0.0f), dirLightV, glm::vec3(1.0f, 1.0f, 1.0f), 0.3f);
	lightSource point(glm::vec3(-5.5f, 1.5f, -0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f);
	point.visualize(true);
	dir.visualize(true);

	glfwSetTime(0);

	glEnable(GL_DEPTH_TEST);

#ifndef LEGACY_INPUT_SYSTEM
	input::init(mainWindow);
#endif
	while (!glfwWindowShouldClose(mainWindow)) {
		if (engine::engineTerminated()) {
			std::cout << std::endl;
			std::cout << "ERROR: LMGE: ENGINE TERMINATED." << std::endl;
			return -1;
		}

		currentTime = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef LEGACY_INPUT_SYSTEM
		input::update();	//updates the recorded key presses of the last frame
		if (input::getNewPress("f")) mouseInput = !mouseInput;

#ifdef DEBUG_INPUT_SYSTEM
		input::activeScan();
#endif
#endif

		if (input::getMouse("mouse_right")) grav = true;
		else grav = false;
		//hide cursor and set the callback for processing input when mouseInput == true
		if (mouseInput) {
			glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(mainWindow, mouseMovementCallback);
		}
		else {
			glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(mainWindow, nullptr);
		}

		//getting the view matrix which must be updated each frame sice camPos and camFront can change at runtime
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(camPos, camPos + camFront, glm::vec3(0.0f, 1.0f, 0.0f));

		glDisable(GL_CULL_FACE);
		if(activeSkybox != nullptr) activeSkybox->update(viewMatrix, projectionMatrix);

		engine::getRenderShader()->use();

		//setting the data for the matricesBlock uniform buffer
		//each matrix takes 64 bytes
		glBindBuffer(GL_UNIFORM_BUFFER, matricesBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(viewMatrix));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(projectionMatrix));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

#ifndef DISABLE_LIGHTING
		for (lightSource* light : allLights) {
			light->bind(*engine::getRenderShader(), camPos);
		}
		engine::getRenderShader()->uniform(float1, "ambientStrength", {0.6f});
#endif
		glEnable(GL_CULL_FACE);

		engine::getRenderShader()->uniformBlock("matrices", 0);

		for (int i = 0; i < allObjects.size(); i++) {
			allObjects[i]->update();
		}

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();

		for (int i = 0; i < allObjects.size(); i++)	allObjects[i]->postFrameCleanup();
		lightSource::postFrameCleanup();

		deltaTime = glfwGetTime() - currentTime;
	}
	glfwTerminate();

	return 0;
}