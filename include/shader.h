#pragma once
#include "fundamental.h"

enum uniformType {uint1, int1, float1, float2, float3, float4};

class shader {
private:
	unsigned int vShader, gShader, fShader;
	unsigned int program;

	std::string getData(std::string path);

public:
	void init(const std::string vShaderPath, const std::string fShaderPath);
	void init(const std::string vShaderPath, const std::string gShaderPath, const std::string fShaderPath);
	shader();
	shader(const std::string vShaderPath, const std::string fShaderPath);
	shader(const std::string vShaderPath, const std::string gShaderPath, const std::string fShaderPath);

	void uniform(uniformType type, std::string name, std::vector<float> value);
	void uniform(std::string name, glm::mat4 value);
	void uniform(std::string name, glm::mat3 value);
	void uniformBlock(std::string name, unsigned int binding);

	void use();
};

//----------For light source class---------
extern unsigned int numActiveDirLight, numActivePointLight;

class lightSource {
private:
	glm::vec3 dir;
	glm::vec3 pos;
	glm::vec3 color;
	float ambientStrength;

	bool directional = false;
	bool visualizationEnabled = false;
	bool renderingDataInitialized = false;

	void(*updateFunction)(void) = nullptr;

	unsigned int renderingVAO, renderingVBO;
	shader renderingShader;
public:
	lightSource(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float ambient);
	void makeDynamic(void(*updateFunction)(void));
	void bind(shader& currentShader);
	void bind(shader& currentShader, glm::vec3 camPos);

	void changePos(glm::vec3 value);
	void changeColor(glm::vec3 value);
	void shiftPos(glm::vec3 movementVector);
	void shiftColor(glm::vec3 vector);

	void visualize(bool val);

	static void postFrameCleanup();
};
extern std::vector<lightSource*> allLights;