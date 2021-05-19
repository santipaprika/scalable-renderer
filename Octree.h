#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "glm/glm.hpp"
#include "Plane.h"

using namespace std;

enum RepresentativeMode { AVG, QEM };
enum ClutseringMode { VOXEL, VOXEL_AND_NORMAL };

class Octree
{
private:
    glm::vec3 minAABB;
    float halfLength;

    glm::vec3 addedPosition;
    glm::vec3 position;
    int maxDepth;
    int nVertices;
    int idx;

    // QEM
    vector<Plane*> quadrics;

    // NORMAL CLUSTERING
    vector<Plane*> clusteredQuadrics[8];
    glm::vec3 clusterAddedPosition[8];

    static int counter;

public:
    // Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent = nullptr);
    Octree(int maxDepth, glm::vec3 minAABB, float halfLength, Octree *parent = nullptr);
    ~Octree();

    glm::vec3 getPosition() const;
    Octree* evaluateVertexQEM(const glm::vec3 &vertex, unordered_map<int, unordered_set<Plane *>> &vertexToQuadric, unordered_map<int, unordered_set<int>> &vertexToNormalCluster,
                            int idx=0);
    Octree* evaluateVertexAVG(const glm::vec3 &vertex, unordered_map<int, unordered_set<int>> &vertexToNormalCluster, int idx=0);
    int getIndex() const;
    void computeMeanPositions();
    void computeMeanNClusterPositions();
    void computeQEMPositions();
    void computeQEMNClusterPositions();
    void computeRepresentatives();

    int nClusteredVertices[8];
    glm::vec3 clusteredRepresentatives[8];
    Octree *childs[8];
    Octree *parent;
};

#endif // __OCTREE_H__