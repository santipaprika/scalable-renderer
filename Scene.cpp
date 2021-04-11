#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"
#include "Application.h"

Scene::Scene()
{
    mesh = NULL;
}

Scene::~Scene()
{
    if (mesh != NULL)
        delete mesh;
}

void Scene::init()
{
    initShaders();
    mesh = new TriangleMesh();
    mesh->buildCube();
    mesh->sendToOpenGL(basicProgram);
    currentTime = 0.0f;

    camera.init(-2.0f);
    meshInstances_dim1 = 2;
    setupMuseumScene();

    bPolygonFill = true;
}

void Scene::setupGridScene()
{
    for (int i = 0; i < nodes.size(); i++)
    {
        delete nodes[i];
    }
    nodes.clear();

    float x_step = mesh->getExtents().r * 5 / 4;
    float y_step = mesh->getExtents().g * 5 / 4;

    for (float i = x_step / 2 * (-meshInstances_dim1 + 1); i <= (meshInstances_dim1 + x_step) / 2; i += x_step)
    {
        for (float j = y_step / 2 * (-meshInstances_dim1 + 1); j <= (meshInstances_dim1 + y_step) / 2; j += y_step)
        {
            glm::mat4 model(1.0);
            model = glm::translate(model, glm::vec3(i, 0, j));
            nodes.push_back(new Node(mesh, model));
        }
    }
}

void Scene::setupMuseumScene()
{
    std::ifstream tilemap("../tilemap.tmx");

    // get first 5 lines (headers)
    std::string lineStr;
    int gridWidth, gridHeight;
    for (int i = 0; i < 5; i++)
    {
        std::getline(tilemap, lineStr);

        // parse 3rd line (which contiains width/height information)
        if (i == 3)
        {
            std::string word;
            std::stringstream lineSS = std::stringstream(lineStr);
            while (!lineSS.eof())
            {
                lineSS >> word;
                if (word.find("width=") != std::string::npos)
                {
                    gridWidth = stoi(std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1")));
                }
                if (word.find("height=") != std::string::npos)
                {
                    gridHeight = stoi(std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1")));
                }
            }
        }
    }

    // fill grid
    int grid[gridWidth][gridHeight];
    for (int i = 0; i < gridHeight; i++)
    {
        std::getline(tilemap, lineStr);
        // std::cout << lineStr;
        std::stringstream ss(lineStr);
        int num_label;
        for (int j = 0; j < gridWidth; j++)
        {
            ss >> num_label;
            grid[i][j] = num_label;
            if(ss.peek() == ',') ss.ignore();
        }
    }

    tilemap.close();


    float x_step = mesh->getExtents().r * 5 / 4;
    float y_step = mesh->getExtents().g * 5 / 4;

    int x = 0;
    for (float i = x_step / 2 * (-gridWidth + 1); i <= (gridWidth + x_step) / 2; i += x_step)
    {
        int y = -1;
        for (float j = y_step / 2 * (-gridHeight + 1); j <= (gridHeight + y_step) / 2; j += y_step)
        {
            y++;
            // if (grid[90][48] != 1)
            //     std::cout << x << "," << y << ": " << grid[x][y] << std::endl;

            if (grid[x][y] == 1)
                continue;

            glm::mat4 model(1.0);
            model = glm::translate(model, glm::vec3(i, 0, j));
            nodes.push_back(new Node(mesh, model));
        }
        x++;
    }
}

bool Scene::loadMesh(const char *filename)
{
    PLYReader reader;

    mesh->free();
    bool bSuccess = reader.readMesh(filename, *mesh);
    if (bSuccess)
        mesh->computeAABB();
    mesh->sendToOpenGL(basicProgram);

    return bSuccess;
}

void Scene::update(float deltaTime)
{
    currentTime += deltaTime;

    updateKeyPressedEvents(deltaTime);
}

void Scene::render()
{
    for (int i = 0; i < nodes.size(); i++)
    {
        Node *node = nodes[i];

        basicProgram.use();
        basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
        basicProgram.setUniformMatrix4f("view", camera.getModelViewMatrix());
        basicProgram.setUniformMatrix4f("model", node->getModel());

        basicProgram.setUniform1i("bLighting", bPolygonFill ? 1 : 0);
        if (bPolygonFill)
        {
            basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
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
