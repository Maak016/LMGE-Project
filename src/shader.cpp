#include "shader.h"

unsigned int numActiveDirLight = 0;
unsigned int numActivePointLight = 0;

std::vector<lightSource*> allLights;

lightSource::lightSource(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float ambient) {
	this->dir = dir;
	this->pos = pos;
	this->color = color;
	this->ambientStrength = ambient;

	directional = pos == glm::vec3(0.0f, 0.0f, 0.0f);

	allLights.push_back(this);
}
void lightSource::makeDynamic(void(*updateFunction)(void)) { this->updateFunction = updateFunction; }

void lightSource::bind(shader& currentShader) {
	if (updateFunction != nullptr) updateFunction();

	if (visualizationEnabled && renderingDataInitialized)
		std::cout << "WARNING: Using normal lightSource::bind(shader&) instead of lightSource::bind(shader&, glm::vec3) although light source visualization is enabled. The light source will not be rendered." << std::endl;

	currentShader.use();
	std::string name = directional ? "dirLight[" + std::to_string(numActiveDirLight++) + "]" : "pointLight[" + std::to_string(numActivePointLight++) + "]";

	//binding the dir or pos member based on the light source's detection to be a directional light, thus avoiding uniform binding error
	if (directional) currentShader.uniform(float3, name + ".dir", { dir.x, dir.y, dir.z });
	else currentShader.uniform(float3, name + ".pos", { pos.x, pos.y, pos.z });

	currentShader.uniform(float3, name + ".color", { color.x, color.y, color.z });
	currentShader.uniform(float1, name + ".ambient", { ambientStrength });
}

void lightSource::bind(shader& currentShader, glm::vec3 camPos) {
	if (updateFunction != nullptr) updateFunction();

	//render the light if enabled
	if (visualizationEnabled && renderingDataInitialized) {
		renderingShader.use();

		glBindVertexArray(renderingVAO);

		renderingShader.uniformBlock("matrices", 0);

		glm::mat4 model = glm::mat4(1.0f);

		if (directional) model = glm::translate(model, camPos + (dir * 15.0f));
		else model = glm::translate(model, pos);

		renderingShader.uniform("lightModel", model);
		renderingShader.uniform(float3, "lightColor", { color.x, color.y, color.z });

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	currentShader.use();
	std::string name = directional ? "dirLight[" + std::to_string(numActiveDirLight++) + "]" : "pointLight[" + std::to_string(numActivePointLight++) + "]";

	//binding the dir or pos member based on the light source's detection to be a directional light, thus avoiding uniform binding error
	if (directional) currentShader.uniform(float3, name + ".dir", { dir.x, dir.y, dir.z });
	else currentShader.uniform(float3, name + ".pos", { pos.x, pos.y, pos.z });

	currentShader.uniform(float3, name + ".color", { color.x, color.y, color.z });
	currentShader.uniform(float1, name + ".ambient", { ambientStrength });
}

void lightSource::postFrameCleanup() {
	numActiveDirLight = 0;
	numActivePointLight = 0;
}

void lightSource::visualize(bool val) {
	//if the user sets visualization to false, visualizationEnabled will be set to false and the function will exit, preventing any further code execution
	if (!val) {
		visualizationEnabled = false;
		return;
	}
	//exit the function if VAO initialized has been completed, preventing further excessive process that could harm performance
	if (val && renderingDataInitialized) {
		visualizationEnabled = true;
		return;
	}

	visualizationEnabled = true;
	//setting up rendering data
	const float vertices[] =
	{
		-0.5f, -0.5f, 0.5f,     0.5f, -0.5f, 0.5f,     -0.5f, 0.5f, 0.5f,		//front side    
		0.5f, -0.5f, 0.5f,      -0.5f, 0.5f, 0.5f,     0.5f, 0.5f, 0.5f,

		0.5f, -0.5f, -0.5f,     -0.5f, -0.5f, -0.5f,   0.5f, 0.5f, -0.5f,		//back side
		-0.5f, -0.5f, -0.5f,    0.5f, 0.5f, -0.5f,     -0.5f, 0.5f, -0.5f,

		0.5f, -0.5f, 0.5f,      0.5f, -0.5f, -0.5f,    0.5f, 0.5f, 0.5f,		//right side 
		0.5f, -0.5f, -0.5f,     0.5f, 0.5f, 0.5f,      0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, -0.5f,    -0.5f, -0.5f, 0.5f,    -0.5f, 0.5f, -0.5f,		//left side 
		-0.5f, -0.5f, 0.5f,     -0.5f, 0.5f, -0.5f,    -0.5f, 0.5f, 0.5f,

		-0.5f, 0.5f, 0.5f,      0.5f, 0.5f, 0.5f,      -0.5f, 0.5f, -0.5f,		//top side
		0.5f, 0.5f, 0.5f,       -0.5f, 0.5f, -0.5f,    0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, -0.5f,    0.5f, -0.5f, -0.5f,    -0.5f, -0.5f, 0.5f,		//bottom side
		0.5f, -0.5f, -0.5f,     -0.5f, -0.5f, 0.5f,    0.5f, -0.5f, 0.5f
	};

	glGenVertexArrays(1, &renderingVAO);
	glBindVertexArray(renderingVAO);

	glGenBuffers(1, &renderingVBO);
	glBindBuffer(GL_ARRAY_BUFFER, renderingVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	renderingShader.init("shaders/lightSource.lmv", "shaders/lightSource.lmf");

	renderingDataInitialized = true;
	std::cout << "SUCCESSFUL: Setting up light source object for rendering." << std::endl;
}

void lightSource::changePos(glm::vec3 value) {
	if (directional) dir = value;
	else pos = value;
}
void lightSource::changeColor(glm::vec3 value) { color = value; }
void lightSource::shiftPos(glm::vec3 movementVector) { this->pos += movementVector; }
void lightSource::shiftColor(glm::vec3 vector) { this->color += vector; }

shader::shader(){}

std::string shader::getData(const std::string path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "ERROR: Failed to open shader source file at path: '" << path << "'" << std::endl;
		return "";
	}
	std::stringstream fileData;
	fileData << file.rdbuf();
	return fileData.str();
}

void shader::init(const std::string vShaderPath, const std::string fShaderPath) {
	vShader = glCreateShader(GL_VERTEX_SHADER);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	std::string v = getData(vShaderPath);
	const char* vSrc = v.c_str();
	std::string f = getData(fShaderPath);
	const char* fSrc = f.c_str();

	int status;
	char log[512];

	//Vertex shader
	glShaderSource(vShader, 1, &vSrc, nullptr);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(vShader, 512, nullptr, log);
		std::cout << "ERROR: Vertex shader compilation failed.\n See log for more details:\n" << log << std::endl;
	}

	//Fragment shader
	glShaderSource(fShader, 1, &fSrc, nullptr);
	glCompileShader(fShader);
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(fShader, 512, nullptr, log);
		std::cout << "ERROR: Fragment shader compilation failed.\n See log for more details:\n" << log << std::endl;
	}

	//Program
	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::cout << "ERROR: Shader program linking failed.\n See log for more details:\n" << log << std::endl;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);
}
void shader::init(const std::string vShaderPath, const std::string gShaderPath, const std::string fShaderPath) {
	vShader = glCreateShader(GL_VERTEX_SHADER);
	gShader = glCreateShader(GL_GEOMETRY_SHADER);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	std::string v = getData(vShaderPath);
	const char* vSrc = v.c_str();
	std::string g = getData(gShaderPath);
	const char* gSrc = g.c_str();
	std::string f = getData(fShaderPath);
	const char* fSrc = f.c_str();

	int status;
	char log[512];

	//Vertex shader
	glShaderSource(vShader, 1, &vSrc, nullptr);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(vShader, 512, nullptr, log);
		std::cout << "ERROR: Vertex shader compilation failed.\n See log for more details:\n" << log << std::endl;
	}

	//Geometry shader
	glShaderSource(gShader, 1, &gSrc, nullptr);
	glCompileShader(gShader);
	glGetShaderiv(gShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(gShader, 512, nullptr, log);
		std::cout << "ERROR: Geometry shader compilation failed.\n See log for more details:\n" << log << std::endl;
	}

	//Fragment shader
	glShaderSource(fShader, 1, &fSrc, nullptr);
	glCompileShader(fShader);
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(fShader, 512, nullptr, log);
		std::cout << "ERROR: Fragment shader compilation failed.\n See log for more details:\n" << log << std::endl;
	}

	//Program
	glAttachShader(program, vShader);
	glAttachShader(program, gShader);
	glAttachShader(program, fShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		glGetProgramInfoLog(program, 512, nullptr, log);
		std::cout << "ERROR: Shader program linking failed.\n See log for more details:\n" << log << std::endl;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);
}

shader::shader(const std::string vShaderPath, const std::string fShaderPath) {
	init(vShaderPath, fShaderPath);
}
shader::shader(const std::string vShaderPath, const std::string gShaderPath, const std::string fShaderPath) {
	init(vShaderPath, gShaderPath, fShaderPath);
}

void shader::uniform(uniformType type, std::string name, std::vector<float> value) {
	//Get uniform location
	int location = glGetUniformLocation(program, name.c_str());
	if (location == -1) std::cout << "ERROR: Failed to find uniform with name: '" << name << "'." << std::endl;

	//set new value
	switch (type) {
	case int1: glUniform1i(location, static_cast<int>(value[0])); break;
	case uint1: glUniform1ui(location, static_cast<unsigned int>(value[0])); break;
	case float1: glUniform1f(location, value[0]); break;
	case float2: glUniform2f(location, value[0], value[1]); break;
	case float3: glUniform3f(location, value[0], value[1], value[2]); break;
	case float4: glUniform4f(location, value[0], value[1], value[2], value[3]); break;

	default:
		std::cout << "ERROR: Uniform Setting failed: Invalid uniform type specified." << std::endl;
		break;
	}
}
void shader::uniform(std::string name, glm::mat4 value) {
	int location = glGetUniformLocation(program, name.c_str());
	if (location == -1) std::cout << "ERROR: Failed to find uniform with name: '" << name << "'." << std::endl;

	if (name == "model") {
#ifndef DISABLE_INSTANCING
		std::cout << "ERROR: Legacy model matrix binding used when instancing is enabled." << std::endl;
		return;
#endif
	}

	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
void shader::uniform(std::string name, glm::mat3 value) {
	int location = glGetUniformLocation(program, name.c_str());
	if (location == -1) std::cout << "ERROR: Failed to find uniform with name: '" << name << "'." << std::endl;

	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void shader::uniformBlock(std::string name, unsigned int binding) {
	unsigned int index = glGetUniformBlockIndex(program, name.c_str());

	glUniformBlockBinding(program, index, binding);
}

void shader::use() { glUseProgram(program); }