#include "fundamental.h"
#include "shader.h"
#include "inputSystem.h"
#include "gameObj.h"

double deltaTime = 0;
double currentTime = 0;

const int scrWidth = 1280;
const int scrHeight = 720;

bool mouseInput = false;
bool released = true;

//hitbox region
std::vector<glm::vec3> cube = {
	glm::vec3(-0.3f, 0.3f, 0.5f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.3f, -0.3f, 0.3f), glm::vec3(-0.3f, -0.3f, 0.3f),
	glm::vec3(-0.3f, 0.3f, -0.3f), glm::vec3(0.3f, 0.3f, -0.3f), glm::vec3(0.3f, -0.3f, -0.3f), glm::vec3(-0.3f, -0.3f, -0.3f)
};

bool move = false;
gameObject testingObject;
void setup(shader& s) {
	model backpack("assets/scene2/Castle/Castle OBJ.obj");
	testingObject.init(backpack, s, nullptr, nullptr, { cube });

	testingObject.instantiate(glm::vec3(30.0f, -26.0f, 25.0f), glm::vec3(0.0f, 50.0f, 0.0f));
}

gameObject another;
void setup2(shader& s) {
	model backpack("assets/scene1/backpack/backpack.obj");
	testingObject.init(backpack, s, nullptr, nullptr, { cube });

	testingObject.instantiate(glm::vec3(3.0f, -0.6f, 3.1f), glm::vec3(0.0f, 0.0f, 0.0f));
}


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

	unsigned int matricesBlock;
	glGenBuffers(1, &matricesBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesBlock);
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_STATIC_DRAW);	//2 * matrix4 = 2 * (4 * 4N) = 32N = 128bytes
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

#ifdef DISABLE_LIGHTING
	shader mainShader("shaders/main.lmv", "shaders/noLighting.lmf");
#elif defined(DISABLE_INSTANCING)
	shader mainShader("shaders/noInstancing.lmv", "shaders/main.lmf");
#else
	shader mainShader("shaders/main.lmv", "shaders/main.lmf");
#endif

	//just for testing
	setup2(mainShader);
	//skybox normSkybox;
	//normSkybox.init("assets/scene2/skyboxNorm");

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
	while (!glfwWindowShouldClose(mainWindow)) {
		currentTime = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//For pressing F to lock and hide the mouse for looking around with the mouse
		//the boolean "released" is so that input from the key is not received until it has been released
		if (glfwGetKey(mainWindow, GLFW_KEY_F) && released) {
			mouseInput = !mouseInput;
			released = false;
		}
		else if (!glfwGetKey(mainWindow, GLFW_KEY_F) && !released) released = true;

		inputHandler(mainWindow);

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

		if(activeSkybox != nullptr) activeSkybox->update(viewMatrix, projectionMatrix);

		mainShader.use();

		//setting the data for the matricesBlock uniform buffer
		//each matrix takes 64 bytes
		glBindBuffer(GL_UNIFORM_BUFFER, matricesBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(viewMatrix));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(projectionMatrix));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
#ifndef DISABLE_LIGHTING
		for (lightSource* light : allLights) {
			light->bind(mainShader, camPos);
		}
		mainShader.uniform(float1, "ambientStrength", {0.6f});
#endif

		mainShader.uniformBlock("matrices", 0);

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