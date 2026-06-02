#include "engineGovernor.h"
#include "inputSystem.h"

//namespace engine
bool terminationStatus = false;
void engine::terminate() { terminationStatus = true; }
bool engine::engineTerminated() { return terminationStatus; }

std::vector<void(*)(void)> objectInitFunction;
void engine::declareGameObjectCreation(void(*func)(void)) {
	objectInitFunction.push_back(func);
}
void engine::loadScene() {
	for (const auto& func : objectInitFunction) func();
}
void engine::exception::throwFatalError(const std::string msg) {
	std::cout << "ERROR: " << msg << std::endl;
	engine::terminate();
}

void engine::fileSystem::createObjectScript(const unsigned int scene, const std::string name) {
	std::ofstream header("include/objectScripts/" + name + ".h");
	std::ofstream scriptFile("assets/scene" + std::to_string(scene) + "/" + name + ".cpp");

	std::ifstream headerTemplate("assets/defaultAssets/codingTemplate/objectScriptHeader.lm");
	std::ifstream cppTemplate("assets/defaultAssets/codingTemplate/objectScriptFile.lm");

	if (!header.is_open()) engine::exception::throwFatalError("Failed to open Header file (.h) of object script.");
	if (!scriptFile.is_open()) engine::exception::throwFatalError("Failed to open main code file (.cpp) of object scriptFailed to open main code file (.cpp) of object script");

	if (!headerTemplate.is_open()) engine::exception::throwFatalError("Failed to open Header file template of object script.");
	if (!cppTemplate.is_open()) engine::exception::throwFatalError("Failed to open code file template of object script.");

	std::stringstream headerTemplateDat;
	std::stringstream cppTemplateDat;

	headerTemplateDat << headerTemplate.rdbuf();
	cppTemplateDat << cppTemplate.rdbuf();

	std::string headerString = headerTemplateDat.str();
	std::string cppString = cppTemplateDat.str();

	for (int i = 0; i < headerString.length(); i++) {
		if (headerString[i] != '@') continue;

		headerString = headerString.substr(0, i) + name + headerString.substr(i + 1, headerString.size() - (i + 1));
		i += name.length();
	}
	for (int i = 0; i < cppString.length(); i++) {
		if (cppString[i] != '@') continue;

		cppString = cppString.substr(0, i) + name + cppString.substr(i + 1, cppString.size() - (i + 1));
		i += name.length();
	}

	header << headerString;
	scriptFile << cppString;

	header.close();
	scriptFile.close();
	headerTemplate.close();
	cppTemplate.close();
}