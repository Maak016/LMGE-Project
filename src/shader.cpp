#include "shader.h"

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