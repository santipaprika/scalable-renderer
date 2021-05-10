#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Node.h"


enum Tile {NOTHING, FLOOR, ORIGIN, CUBE, BUNNY, DRAGON, FROG, HAPPY, HORSE, LUCY, MAXPLANCK, MOAI, SPHERE, TETRAHEDRON, TORUS};


// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    void setupGridScene();
    void setupMuseumScene(bool useQEM=false, bool initCamera=true);
    void update(float deltaTime);
    void render();
    void updateKeyPressedEvents(float deltaTime);

    Camera &getCamera();

    void switchPolygonMode();
    void setNumInstances(int numInstances_dim1);
    void clearNodes();
    
    void addNode(TriangleMesh* mesh, glm::vec3 translation=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0));

    ShaderProgram basicProgram;

private:
    void initShaders();
    void computeModelViewMatrix();

private:
    Camera camera;
    TriangleMesh *cube;
    float currentTime;

    bool bPolygonFill;

    int meshInstances_dim1;
    float gridStep;

    std::vector<Node *> nodes;
};

#endif // _SCENE_INCLUDE
