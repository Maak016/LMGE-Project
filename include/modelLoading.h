#pragma once
#include "fundamental.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "shader.h"

enum objects3D {CUBE, SPHERE, PYRAMID };

struct Texture {
	unsigned int ID;
	std::string path;
	std::string name;
};
struct Vertex {
	glm::vec3 coords;
	glm::vec3 normals;
	glm::vec2 texCoords;
};
class Mesh {
private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	unsigned int VAO, VBO, EBO;
public:
	void setup();
	Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures);
	void draw(shader& Shader, glm::mat4 &modelMatrix);
};

class model {
private:
	std::string directory;
	std::vector<Mesh> meshes;
	std::vector<Texture> loadedTextures;
	
	std::vector<Texture> processMaterial(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int importTexture(const std::string path);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	void processNode(aiNode* node, const aiScene* scene);
public:
	void init(std::string path);
	model();
	model(std::string path);
	void modelSimple(objects3D shape, std::string texturePath);
	void modelSimple(objects3D shape);
	void draw(shader& Shader, glm::mat4& modelMatrix);
};