#include "modelLoading.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Mesh::setup() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//VBO setup
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, coords)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normals)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//EBO setup
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setup();
}

void Mesh::draw(shader& Shader, glm::mat4& modelMatrix) {
	Shader.use();
	glBindVertexArray(VAO);

	int diffN = 0;
	int specN = 0;
	for (int i = 0; i < textures.size(); i++) {
		Texture current = textures[i];
		std::string num;
		if (current.name == "tex_diffuse") num = std::to_string(++diffN);
		else if (current.name == "tex_specular") num = std::to_string(++specN);

		Shader.uniform(int1, current.name + num, { static_cast<float>(i) });

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, current.ID);
		num.clear();
	}
	Shader.uniform("model", modelMatrix);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

unsigned int model::importTexture(const std::string path) {
	stbi_set_flip_vertically_on_load(true);

	unsigned int texture;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	std::string fullPath = directory + '/' + path;
	std::cout << "LOADING: " << fullPath << std::endl;

	int x, y, nC;
	unsigned char* data = stbi_load(fullPath.c_str(), &x, &y, &nC, 0);
	if (!data) {
		std::cout << "ERROR: Loading texture file failed from path: '" << fullPath << "'." << std::endl;
	}
	else std::cout << "SUCCESSFUL: " << fullPath << std::endl;

	GLenum format;
	switch (nC) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	default:
		std::cout << "UNEXPECTED ERROR: More than 4 or Fewer than 1 color components in texture loaded from: " << fullPath << std::endl;
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}
std::vector<Texture> model::processMaterial(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> result;

	for (int i = 0; i < mat->GetTextureCount(type); i++) {
		Texture current;

		bool loaded = false;
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path = str.C_Str();

		for (Texture texture : loadedTextures) {
			if (texture.path == path) {
				loaded = true;
				result.push_back(texture);
				break;
			}
		}

		if (!loaded) {
			current.ID = importTexture(path);
			current.path = path;
			current.name = typeName;
			loadedTextures.push_back(current);

			result.push_back(current);
		}
	}

	return result;
}

void model::processNode(aiNode* node, const aiScene* scene) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh model::processMesh(aiMesh* mesh, const aiScene* scene) {
	//processing vertices
	std::vector<Vertex> vertices;
	for (int i = 0; i < mesh->mNumVertices; i++) {
		Vertex current;
		current.coords = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		current.normals = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		current.texCoords = glm::vec2(mesh->mTextureCoords[0]->x, mesh->mTextureCoords[0]->y);

		vertices.push_back(current);
	}

	//processing indices
	std::vector<unsigned int> indices;
	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	//processing textures
	std::vector<Texture> textures;
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

	std::vector<Texture> diffuse = processMaterial(mat, aiTextureType_DIFFUSE, "tex_diffuse");
	textures.insert(textures.end(), diffuse.begin(), diffuse.end());
	std::vector<Texture> specular = processMaterial(mat, aiTextureType_SPECULAR, "tex_specular");
	textures.insert(textures.end(), specular.begin(), specular.end());

	return Mesh(vertices, indices, textures);
}

model::model(){}
model::model(std::string path) {
	directory = path.substr(0, path.find_last_of('/'));

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR: ASSIMP: " << importer.GetErrorString() << std::endl;
	}

	processNode(scene->mRootNode, scene);
	std::cout << "SUCCESSFUL: Assimp node procession from path: " << path << std::endl;
}

void model::init(std::string path) {
	directory = path.substr(0, path.find_last_of('/'));

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR: ASSIMP: " << importer.GetErrorString() << std::endl;
	}

	processNode(scene->mRootNode, scene);
	std::cout << "SUCCESSFUL: Assimp node procession from path: " << path << std::endl;
}

void model::draw(shader& Shader, glm::mat4& modelMatrix) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes.at(i).draw(Shader, modelMatrix);
	}
}