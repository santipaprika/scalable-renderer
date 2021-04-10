#include "Node.h"

Node::Node(TriangleMesh* mesh, glm::mat4 model) 
{
    this->mesh = mesh;
    this->model = model;
}

Node::~Node() 
{
    
}

glm::mat4 &Node::getModel() 
{
    return model;
}

TriangleMesh* Node::getMesh() 
{
    return mesh;
}

glm::vec3 Node::getPosition()
{
    return glm::vec3(model[3][0],model[3][1],model[3][2]);
}

