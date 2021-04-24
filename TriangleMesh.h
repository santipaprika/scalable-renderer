#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE

#include <string>
#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <unordered_map>
#include "ShaderProgram.h"
#include "Octree.h"

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

    void computeLODs(int depth);

    static TriangleMesh *Get(string path);

private:
    vector<glm::vec3> vertices;
    vector<int> triangles;

    GLuint vao;
    GLuint vbo;
    GLint posLocation, normalLocation;

    glm::vec3 minAABB;
    glm::vec3 maxAABB;

    static unordered_map<string, TriangleMesh *> meshes;
};

#endif // _TRIANGLE_MESH_INCLUDE
