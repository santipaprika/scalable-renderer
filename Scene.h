#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Node.h"
#include <queue>


enum Tile {NOTHING, FLOOR, ORIGIN, CUBE, BUNNY, DRAGON, FROG, HAPPY, HORSE, LUCY, MAXPLANCK, MOAI, SPHERE, TETRAHEDRON, TORUS, ARMADILLO};
enum HystheresisMode {ABS_DIST_HYSTHERESIS, REL_DIST_HYSTHERESIS};


// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    void setupGridScene();
    void setupMuseumScene(bool initCamera=true);
    void update(float deltaTime);
    void render();
    void updateKeyPressedEvents(float deltaTime);

    Camera &getCamera();

    void switchPolygonMode();
    void setNumInstances(int numInstances_dim1);
    void increaseAllNodesLOD();
    void decreaseAllNodesLOD();
    void setAllNodesToLOD(int LOD);
    void clearNodes();
    
    void addNode(TriangleMesh* mesh, glm::vec3 translation=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0), glm::vec2 coords = glm::vec2(-1,-1));

    ShaderProgram basicProgram;
    glm::vec2 gridSize;
    float gridStep;

private:
    void initShaders();
    void computeModelViewMatrix();
    void updateLODs();
    void initializeVisibility();
    void initializeNodesLODs();

private:
    Camera camera;
    TriangleMesh *cube;
    float currentTime;

    bool bPolygonFill;

    int meshInstances_dim1;

    std::vector<Node *> nodes;

    // data structure used to know previous LODs used in hystheresis computation. 
    std::unordered_map<Node*, int> nodesLOD;

    float totalCost;

    std::unordered_set<glm::vec2>** visibilityPerCell; 
};

#endif // _SCENE_INCLUDE
