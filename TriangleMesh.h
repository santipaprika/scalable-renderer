#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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

    TriangleMesh* computeLODs(Octree *octree);
    vector<Plane *> generateQuadrics();
    unordered_map<int, unordered_set<Plane *>> associateVerticesToQuadrics();
    unordered_map<int, unordered_set<int>> associateVerticesToNormalClusters(unordered_map<int, unordered_set<Plane *>> vertexToQuadric);

    // get mesh or create (and return) it if path has not been loaded before
    static TriangleMesh *Get(string filename);
    static bool writeLODS(string filename);
    static void clearMeshes();
    TriangleMesh* getPreviousLOD();
    TriangleMesh* getNextLOD();

    vector<int> triangles;
    vector<glm::vec3> vertices;

   private:
    GLuint vao;
    GLuint vbo;
    GLint posLocation, normalLocation;

    glm::vec3 minAABB;
    glm::vec3 maxAABB;

    static unordered_map<string, TriangleMesh*> meshes;

    TriangleMesh* previousLOD;
    TriangleMesh* nextLOD;
};

#endif  // _TRIANGLE_MESH_INCLUDE
