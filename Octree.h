#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "glm/glm.hpp"
#include "Plane.h"
#include <Eigen/Core>

using namespace std;

enum RepresentativeMode { AVG, QEM };
enum ClutseringMode { VOXEL, VOXEL_AND_NORMAL };

// octree representation for all clustering and metric modes in LOD generation.
class Octree
{
private:

    int maxDepth;
    int nVertices;
    int idx;
    
    glm::vec3 addedPosition;    // could be avoided with mean position computed on the fly.
    glm::vec3 position;

    // QEM
    Eigen::Matrix4d Q;

    // NORMAL CLUSTERING
    Eigen::Matrix4d NormalQ[8];
    glm::vec3 clusterAddedPosition[8];  // could be avoided with mean position computed on the fly.

    static int counter;

public:
    Octree(int maxDepth, Octree *parent = nullptr);
    ~Octree();

    glm::vec3 getPosition() const;
    Octree* evaluateVertexQEM(const glm::vec3 &vertex, unordered_map<int, unordered_set<Plane *>> &vertexToQuadric, vector<unordered_set<Plane*>> &octreeToQuadric,
                            unordered_map<int, unordered_set<int>> &vertexToNormalCluster, glm::vec3 minAABB, float halfLength, int idx=0);
    Octree* evaluateVertexAVG(const glm::vec3 &vertex, unordered_map<int, unordered_set<int>> &vertexToNormalCluster, glm::vec3 minAABB, float halfLength, int idx=0);
    int getIndex() const;
    void computeMeanPositions();
    void computeMeanNClusterPositions();
    void computeQEMPositions();
    void computeQEMNClusterPositions();
    void computeRepresentatives();

    int nClusteredVertices[8];
    glm::vec3 clusteredRepresentatives[8];
    Octree *childs[8];
    Octree* parent;
};

#endif // __OCTREE_H__