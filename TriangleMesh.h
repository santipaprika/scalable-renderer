#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Octree.h"
#include "Plane.h"
#include "ShaderProgram.h"
#include "glm/gtx/hash.hpp"

#define OUT

using namespace std;

// Class TriangleMesh renders a very simple room with textures

class TriangleMesh {
   public:
    TriangleMesh();

    void addVertex(const glm::vec3 &position);
    void addTriangle(int v0, int v1, int v2);
    void initializeMesh();

    void buildCube();

    void sendToOpenGL(ShaderProgram &program);
    void render() const;
    void free();

    void computeAABB();
    glm::vec3 getExtents() const;

    TriangleMesh *computeLODs(Octree *octree);
    vector<Plane *> generateQuadrics();
    unordered_map<int, unordered_set<Plane *>> associateVerticesToQuadrics();

    static TriangleMesh *Get(string path);

    vector<int> triangles;
    vector<glm::vec3> vertices;

   private:
    GLuint vao;
    GLuint vbo;
    GLint posLocation, normalLocation;

    glm::vec3 minAABB;
    glm::vec3 maxAABB;

    static unordered_map<string, TriangleMesh *> meshes;
};

#endif  // _TRIANGLE_MESH_INCLUDE
