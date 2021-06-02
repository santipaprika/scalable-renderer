#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"
#include "Application.h"
#include "Utils.h"
#include "Color.h"

#define OUT

Scene::Scene()
{
    cube = NULL;
}

Scene::~Scene()
{
    if (cube != NULL)
        delete cube;
    clearNodes();
}

void Scene::init()
{
    initShaders();
    cube = new TriangleMesh();
    cube->buildCube();
    cube->sendToOpenGL(basicProgram);
    currentTime = 0.0f;
    totalCost = 0;

    camera.init(glm::vec3(0, 0, -2));
    meshInstances_dim1 = 2;
    gridStep = 1.5;
    setupMuseumScene();

    updateValueHeap();

    bPolygonFill = true;
}

void Scene::setupGridScene()
{
    clearNodes();

    float x_step = cube->getExtents().r * 5 / 4;
    float y_step = cube->getExtents().g * 5 / 4;

    for (float i = x_step / 2 * (-meshInstances_dim1 + 1); i <= (meshInstances_dim1 + x_step) / 2; i += x_step)
    {
        for (float j = y_step / 2 * (-meshInstances_dim1 + 1); j <= (meshInstances_dim1 + y_step) / 2; j += y_step)
        {
            glm::mat4 model(1.0);
            model = glm::translate(model, glm::vec3(i, 0, j));
            nodes.push_back(new Node(cube, model));
        }
    }
}

void Scene::setupMuseumScene(bool initCamera)
{
    clearNodes();

    std::ifstream tilemap("../tilemap.tmx");
    string modelsPath[12] = {"bunny.ply", "dragon.ply", "frog.ply", "happy.ply", "horse.ply", "lucy.ply", "maxplanck.ply", "moai.ply", "sphere.ply", "tetrahedron.ply", "torus.ply", "Armadillo.ply"};
    glm::vec2 surroundingDirs[4] = {glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(1, 0), glm::vec2(0, 1)};

    glm::vec2 gridSize = Utils::getGridSize(tilemap);

    // initialize grid
    int **grid;
    Utils::initializePointerMatrix(grid, gridSize.x, gridSize.y);

    // fill grid
    Utils::parseGrid(tilemap, OUT grid, gridSize);

    tilemap.close();

    int y = 0;
    for (float i = gridStep / 2 * (-gridSize.y + 1); i <= (gridSize.y * gridStep) / 2; i += gridStep)
    {
        int x = -1;
        for (float j = gridStep / 2 * (-gridSize.x + 1); j <= (gridSize.x * gridStep) / 2; j += gridStep)
        {
            x++;

            if (grid[y][x] == Tile::NOTHING)
                continue;

            glm::mat4 model(1.0);

            // floor
            addNode(cube, glm::vec3(j, -gridStep / 2.0f, i), glm::vec3(1, 0.01, 1));

            // camera origin
            if (initCamera && grid[y][x] == Tile::ORIGIN)
                camera.init(glm::vec3(j, 1, i), 0, 0);

            // check for walls
            for (glm::vec2 dir : surroundingDirs)
            {
                if (grid[y + (int)dir.y][x + (int)dir.x] == Tile::NOTHING)
                {
                    addNode(cube, glm::vec3(j + gridStep / 2.0f * dir.x, gridStep * 1, i + gridStep / 2.0f * dir.y),
                                         glm::vec3(1.0 - abs(dir.x) * 0.99, 3.0, 1.0 - abs(dir.y) * 0.99));
                }
            }


            // model
            TriangleMesh *mesh;
            if (grid[y][x] > Tile::ORIGIN)
            {
                if (grid[y][x] == Tile::CUBE)
                    mesh = cube;
                else
                    mesh = TriangleMesh::Get("../models/" + modelsPath[grid[y][x] - (Tile::CUBE + 1)]);

                addNode(mesh, glm::vec3(j, 0, i));
            }
        }
        y++;
    }

    Utils::deletePointerMatrix(grid, gridSize.x, gridSize.y);
}


void Scene::update(float deltaTime)
{
    currentTime += deltaTime;
    initializeValueHeap();

    updateKeyPressedEvents(deltaTime);
}

void Scene::render()
{
    for (Node *node : nodes)
    {
        basicProgram.use();
        basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
        basicProgram.setUniformMatrix4f("view", camera.getInvModelViewMatrix());
        basicProgram.setUniformMatrix4f("model", node->getModel());

        basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
        if (bPolygonFill)
        {
            glm::vec3 color(0.9f, 0.9f, 0.9f);
            if (node->getMesh()->LODidx == 0) color = Color::red(); 
            if (node->getMesh()->LODidx == 1) color = Color::redyellow(); 
            if (node->getMesh()->LODidx == 2) color = Color::yellow(); 
            if (node->getMesh()->LODidx == 3) color = Color::yellowgreen(); 
            if (node->getMesh()->LODidx >= 4) color = Color::green(); 

            basicProgram.setUniform4f("color", color.x, color.y, color.z, 1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(0.5f, 1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            node->getMesh()->render();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_POLYGON_OFFSET_FILL);
            basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
        }
        node->getMesh()->render();
    }
}

void Scene::updateKeyPressedEvents(float deltaTime)
{
    camera.sprint = Application::instance().getKey(' ');

    enum
    {
        LEFT_KEY = 'a',
        RIGHT_KEY = 'd',
        FRONT_KEY = 'w',
        BACK_KEY = 's',
        UP_KEY = 'e',
        DOWN_KEY = 'q'
    };
    enum
    {
        LEFT,
        RIGHT,
        FRONT,
        BACK,
        UP,
        DOWN
    };
    glm::vec3 directions[] = {glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, 1, 0)};
    glm::vec3 move_direction = glm::vec3(0, 0, 0);

    if (Application::instance().getKey(LEFT_KEY))
        move_direction += directions[LEFT];
    if (Application::instance().getKey(RIGHT_KEY))
        move_direction += directions[RIGHT];
    if (Application::instance().getKey(FRONT_KEY))
        move_direction += directions[FRONT];
    if (Application::instance().getKey(BACK_KEY))
        move_direction += directions[BACK];
    if (Application::instance().getKey(UP_KEY))
        move_direction += directions[UP];
    if (Application::instance().getKey(DOWN_KEY))
        move_direction += directions[DOWN];

    if (move_direction == glm::vec3(0, 0, 0))
        return;

    glm::normalize(move_direction);
    camera.move(move_direction * deltaTime);
}

Camera &Scene::getCamera()
{
    return camera;
}

void Scene::switchPolygonMode()
{
    bPolygonFill = !bPolygonFill;
}

void Scene::setNumInstances(int numInstances_dim1)
{
    this->meshInstances_dim1 = numInstances_dim1;
    setupGridScene();
}

void Scene::increaseAllNodesLOD() 
{
    for (Node* node : nodes) {
        node->useNextLod();
    }
}

void Scene::decreaseAllNodesLOD() 
{
    for (Node* node : nodes) {
        node->usePreviousLod();
    }
}

void Scene::clearNodes()
{
    for (Node *node : nodes)
        delete node;

    nodes.clear();
}

void Scene::addNode(TriangleMesh *mesh, glm::vec3 translation, glm::vec3 scale)
{
    float scaleFactor = gridStep / Utils::max3(mesh->getExtents());

    glm::mat4 model(1.0f);
    model = glm::translate(model, translation);
    model = glm::scale(model, glm::vec3(scaleFactor));
    model = glm::scale(model, scale);
    nodes.push_back(new Node(mesh, model));
}

void Scene::initShaders()
{
    Shader vShader, fShader;

    vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
    if (!vShader.isCompiled())
    {
        cout << "Vertex Shader Error" << endl;
        cout << "" << vShader.log() << endl
             << endl;
    }
    fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
    if (!fShader.isCompiled())
    {
        cout << "Fragment Shader Error" << endl;
        cout << "" << fShader.log() << endl
             << endl;
    }
    basicProgram.init();
    basicProgram.addShader(vShader);
    basicProgram.addShader(fShader);
    basicProgram.link();
    if (!basicProgram.isLinked())
    {
        cout << "Shader Linking Error" << endl;
        cout << "" << basicProgram.log() << endl
             << endl;
    }
    basicProgram.bindFragmentOutput("outColor");
    vShader.free();
    fShader.free();
}

void Scene::initializeValueHeap() 
{
    totalCost = 0;
    glm::vec3 viewpoint = camera.getPosition();
    for (Node* node : nodes) {
        if (!node->getMesh()->hasLODs())
            continue;
        
        node->useLowestLod();
        node->computeBenefit(viewpoint);
        nodesValueHeap.push(node);
    }

    while (!nodesValueHeap.empty()) {
        Node* bestNode = nodesValueHeap.top(); 
        if (bestNode->getMesh()->getNextLOD()->getCost() + totalCost < Application::instance().TPS / 60.f) {
            bestNode->useNextLod();
            nodesValueHeap.pop();
            totalCost += bestNode->getMesh()->getCost(); 
            
            // Add new level to heap only if it exists
            if (bestNode->getMesh()->getCost() == 0)
                continue;
            
            bestNode->computeBenefit(viewpoint);
            nodesValueHeap.push(bestNode);
        } else
            nodesValueHeap.pop();
    }

    // clear
    while (!nodesValueHeap.empty())
        nodesValueHeap.pop();
}

void Scene::updateValueHeap() 
{
    
}
