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

gameObject testingObject;
void setup(shader& s) {
	model backpack("assets/scene1/backpack/backpack.obj");
	testingObject.init(backpack, s, nullptr, nullptr, { {} });

	testingObject.instantiate(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(30.0f, 20.0f, 1.0f));

	allObjects.push_back(&testingObject);
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
	glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_STATIC_DRAW); 
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	shader mainShader("shaders/main.lmv", "shaders/main.lmf");

	setup(mainShader);

	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective(FOV, (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);

	//runs the one time initialization code of each gameObject object.
	for (int i = 0; i < allObjects.size(); i++) allObjects[i]->runtimeInit();

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(mainWindow)) {
		currentTime = glfwGetTime();

		glClearColor(0.0f, 0.0f, 0.2f, 0.3f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetKey(mainWindow, GLFW_KEY_F) && released) {
			mouseInput = !mouseInput;
			released = false;
		}
		else if (!glfwGetKey(mainWindow, GLFW_KEY_F) && !released) released = true;

		inputHandler(mainWindow);

		if (mouseInput) {
			glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(mainWindow, mouseMovementCallback);
		}
		else {
			glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(mainWindow, nullptr);
		}

		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::lookAt(camPos, camPos + camFront, glm::vec3(0.0f, 1.0f, 0.0f));

		mainShader.use();
		glBindBuffer(GL_UNIFORM_BUFFER, matricesBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(viewMatrix));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(projectionMatrix));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		mainShader.uniformBlock("matrices", 0);

		for (int i = 0; i < allObjects.size(); i++) {
			allObjects[i]->update();
		}

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();


		//std::cout << camPos.x << ' ' << camPos.y << ' ' << camPos.z << std::endl;

		for (int i = 0; i < allObjects.size(); i++)	allObjects[i]->postFrameCleanup();

		deltaTime = glfwGetTime() - currentTime;
	}
	glfwTerminate();

	return 0;
}