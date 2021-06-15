#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.h"
#include "Color.h"
#include "PLYReader.h"
#include "Scene.h"
#include "Utils.h"
#include "VisibilityComputer.h"

#define OUT

Scene::Scene() {
    cube = NULL;
}

Scene::~Scene() {
    if (cube != NULL)
        delete cube;
    clearNodes();
}

void Scene::init() {
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
    initializeVisibility();

    initializeNodesLODs();
    updateLODs();

    bPolygonFill = true;
}

void Scene::setupGridScene() {
    clearNodes();

    float x_step = cube->getExtents().r * 5 / 4;
    float y_step = cube->getExtents().g * 5 / 4;

    for (float i = x_step / 2 * (-meshInstances_dim1 + 1); i <= (meshInstances_dim1 + x_step) / 2; i += x_step) {
        for (float j = y_step / 2 * (-meshInstances_dim1 + 1); j <= (meshInstances_dim1 + y_step) / 2; j += y_step) {
            glm::mat4 model(1.0);
            model = glm::translate(model, glm::vec3(i, 0, j));
            nodes.push_back(new Node(cube, model));
        }
    }
}

void Scene::setupMuseumScene(bool initCamera) {
    clearNodes();

    std::ifstream tilemap("../museum/tilemap.tmx");
    string modelsPath[12] = {"bunny.ply", "dragon.ply", "frog.ply", "happy.ply", "horse.ply", "lucy.ply", "maxplanck.ply", "moai.ply", "sphere.ply", "tetrahedron.ply", "torus.ply", "Armadillo.ply"};
    glm::vec2 surroundingDirs[4] = {glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(1, 0), glm::vec2(0, 1)};

    gridSize = Utils::getGridSize(tilemap);

    // initialize grid
    int **grid;
    Utils::initializePointerMatrix<int>(grid, gridSize.x, gridSize.y);

    // fill grid
    Utils::parseGrid(tilemap, OUT grid, gridSize);

    tilemap.close();

    int y = 0;
    for (float i = gridStep / 2 * (-gridSize.y + 1); i <= (gridSize.y * gridStep) / 2; i += gridStep) {
        int x = -1;
        for (float j = gridStep / 2 * (-gridSize.x + 1); j <= (gridSize.x * gridStep) / 2; j += gridStep) {
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
            for (glm::vec2 dir : surroundingDirs) {
                if (grid[y + (int)dir.y][x + (int)dir.x] == Tile::NOTHING) {
                    addNode(cube, glm::vec3(j + gridStep / 2.0f * dir.x, gridStep * 1, i + gridStep / 2.0f * dir.y),
                            glm::vec3(1.0 - abs(dir.x) * 0.99, 3.0, 1.0 - abs(dir.y) * 0.99));
                }
            }

            // model
            TriangleMesh *mesh;
            if (grid[y][x] > Tile::ORIGIN) {
                if (grid[y][x] == Tile::CUBE)
                    mesh = cube;
                else
                    mesh = TriangleMesh::Get("../models/" + modelsPath[grid[y][x] - (Tile::CUBE + 1)]);

                addNode(mesh, glm::vec3(j, 0, i), glm::vec3(1, 1, 1), glm::vec2(y, x));
            }
        }
        y++;
    }

    Utils::deletePointerMatrix<int>(grid, gridSize.x, gridSize.y);
}

void Scene::update(float deltaTime) {
    currentTime += deltaTime;

    if (!Application::instance().bUseFixedLODs)
        updateLODs();

    updateKeyPressedEvents(deltaTime);
}

void Scene::render() {
    glm::vec2 camCoords = camera.getGridCoords();
    std::unordered_set<glm::vec2> visibilitySet = visibilityPerCell[(int)camCoords.y][(int)camCoords.x];

    for (Node *node : nodes) {
        if (Application::instance().bUseVisibility && node->isStatue())
            if (visibilitySet.find(node->getCoords()) == visibilitySet.end())
                continue;

        basicProgram.use();
        basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
        basicProgram.setUniformMatrix4f("view", camera.getInvModelViewMatrix());
        basicProgram.setUniformMatrix4f("model", node->getModel());

        basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
        if (bPolygonFill) {
            glm::vec3 color(0.9f, 0.9f, 0.9f);

            if (Application::instance().bShowColoredLODs) {
                if (node->getMesh()->LODidx == Application::instance().minLODLevel) color = Color::red();
                if (node->getMesh()->LODidx == Application::instance().minLODLevel + 1) color = Color::redyellow();
                if (node->getMesh()->LODidx == Application::instance().minLODLevel + 2) color = Color::yellow();
                if (node->getMesh()->LODidx == Application::instance().minLODLevel + 3) color = Color::yellowgreen();
                if (node->getMesh()->LODidx >= Application::instance().minLODLevel + 4) color = Color::green();
            }

            basicProgram.setUniform4f("color", color.x, color.y, color.z, 1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
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

void Scene::updateKeyPressedEvents(float deltaTime) {
    camera.sprint = Application::instance().getKey(' ');

    enum {
        LEFT_KEY = 'a',
        RIGHT_KEY = 'd',
        FRONT_KEY = 'w',
        BACK_KEY = 's',
        UP_KEY = 'e',
        DOWN_KEY = 'q'
    };
    enum {
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

Camera &Scene::getCamera() {
    return camera;
}

void Scene::switchPolygonMode() {
    bPolygonFill = !bPolygonFill;
}

void Scene::setNumInstances(int numInstances_dim1) {
    this->meshInstances_dim1 = numInstances_dim1;
    setupGridScene();
}

void Scene::increaseAllNodesLOD() {
    for (Node *node : nodes) {
        node->useNextLod();
    }
}

void Scene::decreaseAllNodesLOD() {
    for (Node *node : nodes) {
        node->usePreviousLod();
    }
}

void Scene::setAllNodesToLOD(int LOD) {
    for (Node *node : nodes) {
        if (!node->getMesh()->hasLODs())
            continue;

        while (node->getMesh()->LODidx > LOD)
            node->usePreviousLod();

        while (node->getMesh()->LODidx < LOD)
            node->useNextLod();
    }
}

void Scene::clearNodes() {
    for (int i=0; i<nodes.size(); i++)
        delete nodes[i];

    nodes.clear();
}

void Scene::addNode(TriangleMesh *mesh, glm::vec3 translation, glm::vec3 scale, glm::vec2 coords) {
    float scaleFactor = gridStep / Utils::max3(mesh->getExtents());

    glm::mat4 model(1.0f);
    model = glm::translate(model, translation);
    model = glm::scale(model, glm::vec3(scaleFactor));
    model = glm::scale(model, scale);
    nodes.push_back(new Node(mesh, model, coords));
}

void Scene::initShaders() {
    Shader vShader, fShader;

    vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
    if (!vShader.isCompiled()) {
        cout << "Vertex Shader Error" << endl;
        cout << "" << vShader.log() << endl
             << endl;
    }
    fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
    if (!fShader.isCompiled()) {
        cout << "Fragment Shader Error" << endl;
        cout << "" << fShader.log() << endl
             << endl;
    }
    basicProgram.init();
    basicProgram.addShader(vShader);
    basicProgram.addShader(fShader);
    basicProgram.link();
    if (!basicProgram.isLinked()) {
        cout << "Shader Linking Error" << endl;
        cout << "" << basicProgram.log() << endl
             << endl;
    }
    basicProgram.bindFragmentOutput("outColor");
    vShader.free();
    fShader.free();
}

// Time critical rendering work performed here
void Scene::updateLODs() {
    totalCost = 0;
    std::priority_queue<Node *, vector<Node*>, NodePtrLess> nodesValueHeap;

    glm::vec3 viewpoint = camera.getPosition();
    glm::vec2 camCoords = camera.getGridCoords();
    std::unordered_set<glm::vec2> visibilitySet = visibilityPerCell[(int)camCoords.y][(int)camCoords.x];
    
    // select candidate nodes to evaluate for time critical rendering
    for (Node *node : nodes) {
        if (!node->isStatue())
            continue;

        // do not take into account walls nor hidden nodes to decide LODs.
        if (Application::instance().bUseVisibility)
            if (visibilitySet.find(node->getCoords()) == visibilitySet.end())
                continue;

        // if a node's LOD has been changed recently, keep same LOD without evaluating it.
        if (Application::instance().bUseHystheresis && node->isBlocked()) {
            float distNodeToCamera = glm::distance(node->getPosition(), camera.getPosition());
            int hystheresisMode = Application::instance().hystheresisMode;
            float hystheresisFactor = Application::instance().hystheresisFactor;
            bool keepBlocking = (hystheresisMode == ABS_DIST_HYSTHERESIS) ? (abs(distNodeToCamera - node->getBlockedDistance()) < hystheresisFactor) : abs(distNodeToCamera - node->getBlockedDistance()) < node->getBlockedDistance() * hystheresisFactor;

            if (keepBlocking) {
                totalCost += node->getMesh()->getTotalCost();
                continue;
            }
            node->setBlocked(false);
        }

        node->useLowestLod();
        totalCost += node->getMesh()->getTotalCost();
        node->computeBenefit(viewpoint);
        nodesValueHeap.push(node);
    }

    // upgrade iterativelly the best node's LODs (with more value) until maximum cost is reached.
    while (!nodesValueHeap.empty()) {
        Node *bestNode = nodesValueHeap.top();
        if (bestNode->getMesh()->getCost() + totalCost < Application::instance().TPS / 60.f) {
            totalCost += bestNode->getMesh()->getCost();  // difference in triangles between current and next LOD
            bestNode->useNextLod();
            nodesValueHeap.pop();

            // Add new level to heap only if it exists
            if (bestNode->getMesh()->getCost() == 0)
                continue;

            bestNode->computeBenefit(viewpoint);
            nodesValueHeap.push(bestNode);
        } else
            nodesValueHeap.pop();
    }

    // next step can be ignored if not using hystheresis.
    if (!Application::instance().bUseHystheresis)
        return;

    // Check which nodes' LODs have changed w.r.t previous frame
    for (Node *node : nodes) {
        // do not take into account walls nor hidden nodes to decide LODs.
        if (!node->isStatue()) continue;

        if (visibilitySet.find(node->getCoords()) == visibilitySet.end())
            continue;

        // If it has changed, block and update!
        if (node->getMesh()->LODidx != nodesLOD[node]) {
            node->setBlocked(true);
            node->setBlockedDistance(glm::distance(node->getPosition(), camera.getPosition()));
            nodesLOD[node] = node->getMesh()->LODidx;
        }
    }
}

void Scene::initializeVisibility() {
    Utils::initializePointerMatrix<std::unordered_set<glm::vec2>>(visibilityPerCell, gridSize.x, gridSize.y);
    std::ifstream visibilityFile("visibility.vis");

    if (!visibilityFile.is_open()) {
        VisibilityComputer::computeAndSaveVisibility("../museum/tilemap.tmx");
        visibilityFile.open("visibility.vis");
    }

    string line;
    std::cout << "Reading visibility..." << std::endl;
    int i = 0, j = 0;
    while (std::getline(visibilityFile, line)) {
        std::istringstream iss(line);
        int x, y;

        while (iss >> x >> y) {
            visibilityPerCell[i][j].insert(glm::vec2(y, x));
        }

        j++;
        if (j == gridSize.x) {
            i++;
            j = 0;
        }
    }

    std::cout << "Done!" << std::endl;
}

void Scene::initializeNodesLODs() {
    for (Node *node : nodes)
        nodesLOD[node] = 0;
}
