#include "Node.h"
#include <iostream>

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

TriangleMesh* Node::getMesh() const
{
    return mesh;
}

glm::vec3 Node::getPosition()
{
    return glm::vec3(model[3][0],model[3][1],model[3][2]);
}

void Node::usePreviousLod() 
{
    mesh = mesh->getPreviousLOD();
}

void Node::useNextLod() 
{
    mesh = mesh->getNextLOD();
}

void Node::useLowestLod() 
{
    while (mesh->getPreviousLOD() != mesh)
        mesh = mesh->getPreviousLOD();
}

float Node::getBenefit() const
{
    return benefit;
}

void Node::setBenefit(float benefit) 
{
    this->benefit = benefit;
}

void Node::computeBenefit(glm::vec3 viewpoint) 
{
    glm::vec3 ext = mesh->getExtents();
    float d = glm::length(ext);
    float D = glm::length(getPosition() - viewpoint); 
    benefit = d / (powf(2, mesh->LODidx) * D);
}
