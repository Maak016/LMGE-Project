#pragma once

#include "fundamental.h" 
#include "gameObj.h"

enum feasibleRegionOp {null, less, greater};

struct linearEquation {
	float a;
	float b;
	float c = 1;
};
struct segment {
	glm::vec3 a;
	glm::vec3 b;
};

glm::vec2 solveEquation(float a, float b, float c);
glm::vec2 simulEquation(float x1, float y1, float x2, float y2);

std::vector<segment*> genSegments(std::vector<std::vector<glm::vec3>> vertices);

linearEquation* linearEqFromSegment(segment* s);
bool pointInPolygon(glm::vec3 point, glm::mat4 colliderModelMatrix, std::vector<segment*> shape);
glm::vec2 getIntersection(linearEquation* first, linearEquation* second);

bool separatingAxisTest(gameObject* first, gameObject* second, unsigned int firstIndex, unsigned int secIndex);