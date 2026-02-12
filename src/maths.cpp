#include "fundamental.h"

/* TO DO LIST WITH THIS FILE
*  - 05/02/2025: Fix Memory leak caused by undeleted pointers (dev was too lazy to fix on 5/2/2025 lmao)
*/

glm::vec2 solveEquation(float a, float b, float c) {
	if (a == 0) return glm::vec2(0.0f, 0.0f);

	float delta = b * b - (4 * a * c);
	if (delta < 0) return glm::vec2(0.0f, 0.0f);


	return glm::vec2((-b - sqrt(delta)) / (2 * a), (-b + sqrt(delta)) / (2 * a));
}

glm::vec2 simulEquation(float x1, float y1, float x2, float y2) {
	float D = x1 * 1 - x2 * 1;
	if (D == 0) return glm::vec2(0.0f, 0.0f);

	float Dx = y1 * 1 - y2 * 1;
	float Dy = x1 * y2 - x2 * y1;

	float a = Dx / D;
	float b = Dy / D;

	return glm::vec2(a, b);
}

linearEquation* linearEqFromSegment(segment* s) {
	glm::vec2 ab = simulEquation(s->a.x, s->a.y, s->b.x, s->b.y);

#ifdef DEBUGMODE
	std::cout << "linear eq generation from segment: ";
	std::cout << "segment: (" << s->a.x << ',' << s->a.y << ',' << s->a.z << "), (" << s->b.x << ',' << s->b.y << ',' << s->b.z << ')' << std::endl;
#endif // DEBUGMODE

	

	linearEquation* result = new linearEquation;

	if (s->a.x == s->b.x) {
		result->a = 1;
		result->b = -s->a.x;
		result->c = 0;

		return result;
	}

	result->a = ab.x;
	result->b = ab.y;
	result->c = 1;

	return result;
}
glm::vec2 getIntersection(linearEquation* first, linearEquation* second) {
	float x = (second->b*first->c - first->b*second->c) / (first->a*second->c - second->a*first->c);
	float y = ((first->a * x) + first->b) / first->c;
	return glm::vec2(x, y);
}

/*
* CAUTION: Each of the shape forming the hitbox must have a rectangle or square cross sectional area when slid though along the z-axis.
* The further sides of each 3D shape must be presented in the same order as the near sides
* The number of elements in each shape's vertex array is assumed to be even
* The winding order of the vertices is assumed to be clockwise
*/
std::vector<segment*> genSegments(std::vector<std::vector<glm::vec3>> vertices) {
	std::vector<segment*> result;
	//for each shape that's part of the specified hitbox, each vertex will be paired with the next and one with the same x and y coords to form segments
	for (int i = 0; i < vertices.size(); i++) {
		
		for (int j = 0; j < vertices[i].size() / 2; j++) {
			//pair with the next vertex
			segment* current = new segment;
			current->a = vertices[i].at(j);
			current->b = vertices[i].at(j + 1 >= vertices[i].size() / 2 ? 0 : j+1);

			result.push_back(current);

			if (j == (vertices[i].size() / 2) - 1) break;
			//pair with the corresponding vertex on the further side on the z-axis
			segment* zSegment = new segment;
			zSegment->a = vertices[i].at(j);
			zSegment->b = vertices[i].at(j + vertices[i].size() / 2);

			result.push_back(zSegment);
		}
	}

	return result;
}
//check if the point is in the span of the object's y-coordinates
//a segment becomes part of the return value if within the y coord span
std::vector<segment*> getIntersectingSegments(float y, std::vector<segment*> allSegments) {
	std::vector<segment*> result;

	for (int i = 0; i < allSegments.size(); i++) {
		if (allSegments[i]->a.y >= y && allSegments[i]->b.y <= y) result.push_back(allSegments[i]);
		else if (allSegments[i]->b.y >= y && allSegments[i]->a.y <= y) result.push_back(allSegments[i]);
	}
	return result;
}

/*
* Checks whether or not a point is inside of a polygon by:
* 1. Checking if the point's y-coordinate is within the maximum and minimum y values of the shape
* 2. resetting the rotation of the point and the object to allign the axes (for simpler calculation)
* 3. Forming a line crossing the point and parrallel with the x-axis
* 4. if the number of intersections with the line from the shape (minus the ones on the left of the point) is odd, return true.
* 5. Do the same for the z axis.
*/
bool pointInPolygon(glm::vec3 point, glm::mat4 colliderModelMatrix, std::vector<segment*> shape) {
	bool inXRange = false;
	bool inYRange = false;
	bool inZRange = false;

	///
	//check if the point is in the span of the object's y-coordinates
	///
	float maxY = shape[0]->a.y;
	float minY = shape[0]->b.y;

	for (int i = 0; i < shape.size(); i++) {
		maxY = std::max(shape[i]->a.y, shape[i]->b.y) > maxY ? std::max(shape[i]->a.y, shape[i]->b.y) : maxY;
		minY = std::min(shape[i]->a.y, shape[i]->b.y) < minY ? std::min(shape[i]->a.y, shape[i]->b.y) : minY;
	}

	if (point.y < maxY && point.y > minY) inYRange = true;
	else return false;

#ifdef DEBUGMODE
	std::cout << std::endl << "--Collision system--\n";
	std::cout << "y check:" << ' ' << maxY << ' ' << minY << ' ' << point.y << std::endl;
#endif

	///
	//checking the x-coordinates
	///
	std::vector<glm::vec2> intersections;

	//Inverse of the model matrix for resetting the rotation of the objects to 0 for easier calculation.
	glm::mat4 inverse = glm::inverse(colliderModelMatrix);
	std::vector<segment*> intersectingSegs = getIntersectingSegments(point.y, shape);

	//forming the equation for the line parrellel with the x-axis
	glm::vec3 inversedPoint = glm::vec3(inverse * glm::vec4(point, 1.0f));
	glm::vec2 rightCastEquation = simulEquation(inversedPoint.x, inversedPoint.y, inversedPoint.x + 1.0f, inversedPoint.y);

	linearEquation* castEquation = new linearEquation;
	castEquation->a = rightCastEquation.x;
	castEquation->b = rightCastEquation.y;
	
	//get all the interection coordinates of the point with possible segments (except ones on the right of the point)
	for (int i = 0; i < intersectingSegs.size(); i++) {
		glm::vec3 aInverse = glm::vec3(inverse * glm::vec4(intersectingSegs[i]->a, 1.0f));
		glm::vec3 bInverse = glm::vec3(inverse * glm::vec4(intersectingSegs[i]->b, 1.0f));
		segment* ab = new segment;
		ab->a = aInverse;
		ab->b = bInverse;

		linearEquation* segmentEq = linearEqFromSegment(ab);

		glm::vec2 intersection = getIntersection(castEquation, segmentEq);

		if (intersection.x < inversedPoint.x) continue;	//ignore if the intersection is on the left side of the point
		
#ifdef DEBUGMODE
		//for debugging
		std::cout << "x check:" << std::endl;
		std::cout << "point coordinates:" << inversedPoint.x << ' ' << inversedPoint.y << ' ' << inversedPoint.z << std::endl;
		std::cout << "cast equation: y =" << castEquation->a << 'x' << '+' << castEquation->b << std::endl;
		std::cout << "polygon's segment: (" << ab->a.x << ',' << ab->a.y << ',' << ab->a.z << "), (" << ab->b.x << ',' << ab->b.y << ',' << ab->b.z << ')' << std::endl;
		std::cout << "segment's eq: y = " << segmentEq->a << 'x' << '+' << segmentEq->b << std::endl;
		std::cout << "intersection: (" << intersection.x << ',' << intersection.y << ')' << std::endl;
#endif

		intersections.push_back(intersection);
	}

	//if the number of intersection is even, return false
	//the number of intersection is divided by two because the object contains two side on the z-axis, doubling the number of intersecting segments
	if (intersections.size() == 0 || intersections.size() % 2 == 0) return false;
	inXRange = true;

	intersections.clear();

	///
	//checking the z-coordinates (similar to the x-axis process)
	///
	rightCastEquation = simulEquation(inversedPoint.z, inversedPoint.y, inversedPoint.z + 1.0f, inversedPoint.y);

	castEquation->a = rightCastEquation.x;
	castEquation->b = rightCastEquation.y;

	for (int i = 0; i < intersectingSegs.size(); i++) {
		glm::vec3 aInverse = glm::vec3(inverse * glm::vec4(intersectingSegs[i]->a, 1.0f));
		glm::vec3 bInverse = glm::vec3(inverse * glm::vec4(intersectingSegs[i]->b, 1.0f));
		segment* ab = new segment;
		ab->a = aInverse;
		ab->b = bInverse;

		linearEquation* segmentEq = linearEqFromSegment(ab);

		glm::vec2 intersection = getIntersection(castEquation, segmentEq);

		//operator> used because the coordinates used are of the right-handed system
		if (intersection.x > inversedPoint.z) continue;
		intersections.push_back(intersection);
	}

	if (intersections.size() == 0 || intersections.size() % 2 == 0) return false;
	inZRange = true;

	if (inXRange && inYRange && inZRange) return true;
	return false;
}