#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE

#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Node.h"

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
    Scene();
    ~Scene();

    void init();
    void setupGridScene();
    void setupMuseumScene();
    bool loadMesh(const char *filename);
    void update(float deltaTime);
    void render();
    void updateKeyPressedEvents(float deltaTime);

    Camera &getCamera();

    void switchPolygonMode();

    void setNumInstances(int numInstances_dim1);

private:
    void initShaders();
    void computeModelViewMatrix();

private:
    Camera camera;
    TriangleMesh *mesh;
    ShaderProgram basicProgram;
    float currentTime;

    bool bPolygonFill;

    int meshInstances_dim1;

    std::vector<Node *> nodes;
};

#endif // _SCENE_INCLUDE
