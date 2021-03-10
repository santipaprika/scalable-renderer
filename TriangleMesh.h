#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"


using namespace std;


// Class TriangleMesh renders a very simple room with textures


class TriangleMesh
{

public:
	TriangleMesh();

	void addVertex(const glm::vec3 &position);
	void addTriangle(int v0, int v1, int v2);

	void buildCube();
	
	void sendToOpenGL(ShaderProgram &program);
	void render() const;
	void free();

	void computeAABB();
	glm::vec3 getExtents() const;

private:
  vector<glm::vec3> vertices;
  vector<int> triangles;

	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;

	glm::vec3 minAABB;
	glm::vec3 maxAABB;

	
};


#endif // _TRIANGLE_MESH_INCLUDE


