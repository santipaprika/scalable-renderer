
#include "Octree.h"

#include <iostream>

#include "Application.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

Octree::Octree(int maxDepth, Octree *parent) {
    this->maxDepth = maxDepth;
    this->parent = parent;

    for (int i = 0; i < 8; i++) {
        childs[i] = nullptr;
        nClusteredVertices[i] = 0;
        NormalQ[i] = Eigen::Matrix4d::Zero();
        clusterAddedPosition[i] = glm::vec3(0.f);
        clusteredRepresentatives[i] = glm::vec3(0.f);
    }

    position = glm::vec3(0.0f);
    addedPosition = glm::vec3(0.0f);
    nVertices = 0;

    if (!parent)
        counter = 0;

    idx = counter;
    counter++;

    Q = Eigen::Matrix4d::Zero();
}

int Octree::counter;

Octree::~Octree() {
    for (int i = 0; i < 8; i++)
        if (childs[i])
            delete childs[i];
}

glm::vec3 Octree::getPosition() const {
    return position;
}

Octree *Octree::evaluateVertexQEM(const glm::vec3 &vertex, unordered_map<int, unordered_set<Plane *>> &vertexToQuadric, vector<unordered_set<Plane *>> &octreeToQuadric,
                                  unordered_map<int, unordered_set<int>> &vertexToNormalCluster, glm::vec3 minAABB, float halfLength, int vertexIdx) {
    int clusterMode = Application::instance().clusterMode;
    if (octreeToQuadric.size() <= idx*8) {
        int rep = (clusterMode == VOXEL_AND_NORMAL) ? 8 : 1;
        for (int i = 0; i < rep; i++)
            octreeToQuadric.push_back({});
    }

    if (clusterMode == VOXEL) {
        for (Plane *plane : vertexToQuadric[vertexIdx]) {
            // Add quadric contribution
            if (octreeToQuadric[idx].empty() || octreeToQuadric[idx].find(plane) == octreeToQuadric[idx].end()) {
                Eigen::Vector4d q = Eigen::Vector4d(plane->getPlaneParams());
                Q += q * q.transpose();
                octreeToQuadric[idx].insert(plane);
            }
        }

    } else {  // VOXEL AND NORMALS
        for (Plane *plane : vertexToQuadric[vertexIdx]) {
            for (int i : vertexToNormalCluster[vertexIdx]) {
                if (octreeToQuadric[idx * 8 + i].empty() || octreeToQuadric[idx * 8 + i].find(plane) == octreeToQuadric[idx * 8 + i].end()) {
                    Eigen::Vector4d q = Eigen::Vector4d(plane->getPlaneParams());
                    NormalQ[i] += q * q.transpose();
                    octreeToQuadric[idx * 8 + i].insert(plane);
                }
            }
        }
    }

    // check if has reached maximum depth
    if (maxDepth == 0)
        return this;

    // vertex vector = vertex - AABB center
    glm::vec3 vertexVec = vertex - (minAABB + halfLength);

    // assign vertex to its corresponding subtree
    bool x = vertexVec.x > 0;  // Is at right half
    bool y = vertexVec.y > 0;  // Is at top half
    bool z = vertexVec.z > 0;  // Is at front half

    glm::vec3 minAABBchild = minAABB + glm::vec3(x, y, z) * halfLength;
    if (!childs[z * 4 + y * 2 + x])
        childs[z * 4 + y * 2 + x] = new Octree(maxDepth - 1, this);

    return childs[z * 4 + y * 2 + x]->evaluateVertexQEM(vertex, vertexToQuadric, octreeToQuadric, vertexToNormalCluster, minAABBchild, halfLength/2.f, vertexIdx);
}

Octree *Octree::evaluateVertexAVG(const glm::vec3 &vertex, unordered_map<int, unordered_set<int>> &vertexToNormalCluster, glm::vec3 minAABB, float halfLength, int vertexIdx) {
    int clusterMode = Application::instance().clusterMode;

    if (clusterMode == VOXEL) {
        addedPosition += vertex;
        // add vertex contribution
        nVertices++;

    } else  // VOXEL AND NORMALS
        for (int i : vertexToNormalCluster[vertexIdx]) {
            nClusteredVertices[i]++;
            clusterAddedPosition[i] += vertex;
        }

    // check if has reached maximum depth
    if (maxDepth == 0)
        return this;

    // vertex vector = vertex - AABB center
    glm::vec3 vertexVec = vertex - (minAABB + halfLength);

    // assign vertex to its corresponding subtree
    bool x = vertexVec.x > 0;  // Is at right half
    bool y = vertexVec.y > 0;  // Is at top half
    bool z = vertexVec.z > 0;  // Is at front half

    glm::vec3 minAABBchild = minAABB + glm::vec3(x, y, z) * halfLength;

    if (!childs[z * 4 + y * 2 + x])
        childs[z * 4 + y * 2 + x] = new Octree(maxDepth - 1, this);

    return childs[z * 4 + y * 2 + x]->evaluateVertexAVG(vertex, vertexToNormalCluster, minAABBchild, halfLength/2.f, vertexIdx);
}

int Octree::getIndex() const {
    return idx;
}

void Octree::computeMeanPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeMeanPositions();
    }

    position = addedPosition / (float)nVertices;
}

void Octree::computeMeanNClusterPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeMeanNClusterPositions();

        clusteredRepresentatives[i] = clusterAddedPosition[i] / (float)nClusteredVertices[i];
    }
}

void Octree::computeQEMPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeQEMPositions();
    }

    position = Plane::computePointMinimizingQEM(Q);
}

void Octree::computeQEMNClusterPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeQEMNClusterPositions();

        clusteredRepresentatives[i] = Plane::computePointMinimizingQEM(NormalQ[i]);
    }
}

void Octree::computeRepresentatives() {
    cout << "Computing representatives..." << endl;
    int repMode = Application::instance().repMode;
    int clusterMode = Application::instance().clusterMode;

    switch (repMode) {
        case AVG:
            (clusterMode == VOXEL) ? computeMeanPositions() : computeMeanNClusterPositions();
            break;
        case QEM:
            (clusterMode == VOXEL) ? computeQEMPositions() : computeQEMNClusterPositions();
            break;
        default:
            break;
    }
}
