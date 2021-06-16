#include "TriangleMesh.h"

#include <iostream>

#include "Application.h"
#include "PLYReader.h"
#include "PLYWriter.h"
#include "Scene.h"
#include "Utils.h"

using namespace std;

TriangleMesh::TriangleMesh() {
    previousLOD = nullptr;
    nextLOD = nullptr;
    LODidx = -1;
}

TriangleMesh::~TriangleMesh() 
{
    free();
}

void TriangleMesh::addVertex(const glm::vec3 &position) {
    vertices.push_back(position);
}

void TriangleMesh::addTriangle(int v0, int v1, int v2) {
    triangles.push_back(v0);
    triangles.push_back(v1);
    triangles.push_back(v2);
}

void TriangleMesh::initializeMesh() {
    computeAABB();
    cost = nextLOD ? nextLOD->triangles.size()/3 - triangles.size()/3 : 0;
    sendToOpenGL(Application::instance().scene.basicProgram);
}

void TriangleMesh::buildCube() {
    float vertices[] = {-1, -1, -1,
                        1, -1, -1,
                        1, 1, -1,
                        -1, 1, -1,
                        -1, -1, 1,
                        1, -1, 1,
                        1, 1, 1,
                        -1, 1, 1};

    int faces[] = {3, 1, 0, 3, 2, 1,
                   5, 6, 7, 4, 5, 7,
                   7, 3, 0, 0, 4, 7,
                   1, 2, 6, 6, 5, 1,
                   0, 1, 4, 5, 4, 1,
                   2, 3, 7, 7, 6, 2};

    int i;

    for (i = 0; i < 8; i += 1)
        addVertex(0.5f * glm::vec3(vertices[3 * i], vertices[3 * i + 1], vertices[3 * i + 2]));
    for (i = 0; i < 12; i++)
        addTriangle(faces[3 * i], faces[3 * i + 1], faces[3 * i + 2]);

    computeAABB();
}

void TriangleMesh::sendToOpenGL(ShaderProgram &program) {
    vector<float> data;

    for (unsigned int tri = 0; tri < triangles.size(); tri += 3) {
        glm::vec3 normal;

        normal = glm::cross(vertices[triangles[tri + 1]] - vertices[triangles[tri]],
                            vertices[triangles[tri + 2]] - vertices[triangles[tri]]);
        normal = glm::normalize(normal);
        for (unsigned int vrtx = 0; vrtx < 3; vrtx++) {
            data.push_back(vertices[triangles[tri + vrtx]].x);
            data.push_back(vertices[triangles[tri + vrtx]].y);
            data.push_back(vertices[triangles[tri + vrtx]].z);

            data.push_back(normal.x);
            data.push_back(normal.y);
            data.push_back(normal.z);
        }
    }

    // Send data to OpenGL
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    posLocation = program.bindVertexAttribute("position", 3, 6 * sizeof(float), 0);
    normalLocation = program.bindVertexAttribute("normal", 3, 6 * sizeof(float), (void *)(3 * sizeof(float)));
}

void TriangleMesh::render() const {
    glBindVertexArray(vao);
    glEnableVertexAttribArray(posLocation);
    glEnableVertexAttribArray(normalLocation);
    glDrawArrays((Application::instance().bDrawPoints) ? GL_POINTS : GL_TRIANGLES, 0, 3 * 2 * 3 * triangles.size() / 3);
}

void TriangleMesh::free() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    vertices.clear();
    triangles.clear();
}

void TriangleMesh::computeAABB() {
    minAABB = glm::vec3(INFINITY);
    maxAABB = glm::vec3(-INFINITY);
    for (int i = 0; i < vertices.size(); i++) {
        minAABB.x = min(minAABB.x, vertices[i].x);
        minAABB.y = min(minAABB.y, vertices[i].y);
        minAABB.z = min(minAABB.z, vertices[i].z);

        maxAABB.x = max(maxAABB.x, vertices[i].x);
        maxAABB.y = max(maxAABB.y, vertices[i].y);
        maxAABB.z = max(maxAABB.z, vertices[i].z);
    }
}

glm::vec3 TriangleMesh::getExtents() const {
    return maxAABB - minAABB;
}

TriangleMesh *TriangleMesh::computeLODs(Octree *octree) {
    // contains the quadrics associated to each vertex index
    vector<Octree *> vertexOctree;
    unordered_map<int, unordered_set<int>> vertexToNormalCluster;

    fillOctree(octree, vertexOctree, vertexToNormalCluster);

    // update each node's representative
    octree->computeRepresentatives();

    std::cout << "Creating new LODs..." << std::endl;
    TriangleMesh *LOD = fillLODs(vertexOctree, vertexToNormalCluster);

    delete octree;
    octree = nullptr;

    return LOD;
}

vector<Plane *> TriangleMesh::generateQuadrics() {
    vector<Plane *> facePlane;

    for (int i = 0; i < triangles.size(); i += 3) {
        facePlane.push_back(new Plane(vertices[triangles[i]], vertices[triangles[i + 1]], vertices[triangles[i + 2]]));
    }

    return facePlane;
}

unordered_map<int, unordered_set<Plane *>> TriangleMesh::associateVerticesToQuadrics() {
    // contains the plane containing each face
    vector<Plane *> faceQuadrics = generateQuadrics();

    unordered_map<int, unordered_set<Plane *>> vertexQuadrics;

    for (int i = 0; i < triangles.size(); i++) {
        if (vertexQuadrics.find(triangles[i]) == vertexQuadrics.end()) {
            vertexQuadrics.insert({triangles[i], unordered_set<Plane *>{faceQuadrics[i / 3]}});
        } else {
            vertexQuadrics[triangles[i]].insert(faceQuadrics[i / 3]);
        }
    }

    return vertexQuadrics;
}

unordered_map<int, unordered_set<int>> TriangleMesh::associateVerticesToNormalClusters(unordered_map<int, unordered_set<Plane *>> vertexToQuadric) {
    unordered_map<int, unordered_set<int>> vertexToNormalCluster;

    for (int i = 0; i < vertices.size(); i++) {
        for (Plane *plane : vertexToQuadric[i]) {
            glm::vec3 n = plane->getNormal();
            vertexToNormalCluster[i].insert((n.x > 0) * 4 + (n.y > 0) * 2 + (n.z > 0));
        }
    }

    return vertexToNormalCluster;
}

unordered_map<string, TriangleMesh *> TriangleMesh::meshes = {};

// get mesh or create (and return) it if path has not been loaded before
TriangleMesh *TriangleMesh::Get(string filename) {
    TriangleMesh *mesh = nullptr;
    if (meshes.find(filename) != meshes.end())
        mesh = meshes[filename];
    else if (readLODS(filename))
        mesh = meshes[filename];

    if (mesh) {
        for (int i = 0; i < Application::instance().currentLOD - Application::instance().minLODLevel; i++)
            mesh = mesh->nextLOD;
        return mesh;
    }

    mesh = new TriangleMesh();
    cout << "Reading mesh '" << filename << "'" << endl;
    bool bSuccess = PLYReader::readMesh(filename, (*mesh));
    if (bSuccess) {
        mesh->initializeMesh();

        // create octree
        glm::vec3 center = mesh->minAABB + mesh->getExtents() / 2.0f;
        float maxExtent = max(mesh->getExtents().x, max(mesh->getExtents().y, mesh->getExtents().z));
        glm::vec3 minAABBcube = glm::vec3(center - maxExtent / 2.0f);

        // LODS
        Octree *octree = new Octree(Application::instance().maxLODLevel);

        // compute all LODS and get the lowest
        meshes[filename] = mesh->computeLODs(octree);
        writeLODS(filename);
    } else {
        cout << "E02: Mesh " << filename << " not found!" << endl;
    }
    delete mesh;

    mesh = meshes[filename];
    for (int i = 0; i < Application::instance().currentLOD - Application::instance().minLODLevel; i++)
        mesh = mesh->nextLOD;

    return mesh;
}

bool TriangleMesh::writeLODS(string filename) {
    int minLOD = Application::instance().minLODLevel;
    int maxLOD = Application::instance().maxLODLevel;
    string repModeStr = (Application::instance().repMode == AVG) ? "AVG" : "QEM";
    string clusterModeStr = (Application::instance().clusterMode == VOXEL) ? "VOX" : "VOX-NC";
    TriangleMesh *mesh = meshes[filename];
    for (int i = minLOD; i <= maxLOD; i++) {
        string spec_filename = filename + "_" + repModeStr + "_" + clusterModeStr + "_" + to_string(i) + ".ply";
        if (!PLYWriter::writeMesh(spec_filename, *mesh)) {
            cout << "E0: Could not write " << spec_filename << "." << endl;
            return false;
        }
        mesh = mesh->nextLOD;
    }
    return true;
}

bool TriangleMesh::readLODS(string filename) {
    int minLOD = Application::instance().minLODLevel;
    int maxLOD = Application::instance().maxLODLevel;
    string repModeStr = (Application::instance().repMode == AVG) ? "AVG" : "QEM";
    string clusterModeStr = (Application::instance().clusterMode == VOXEL) ? "VOX" : "VOX-NC";

    TriangleMesh *mesh;
    TriangleMesh *prevMesh;
    for (int i = maxLOD; i >= minLOD; i--) {
        mesh = new TriangleMesh();
        mesh->LODidx = i;

        if (i < maxLOD) {
            mesh->nextLOD = prevMesh;
            prevMesh->previousLOD = mesh;
        }

        bool bSuccess = PLYReader::readMesh(filename + "_" + repModeStr + "_" + clusterModeStr + "_" + to_string(i) + ".ply", (*mesh));
        if (!bSuccess) {
            while (mesh) {
                prevMesh = mesh->nextLOD;
                delete mesh;
                mesh = prevMesh;
            }
            cout << "LOD not found! Generating from scartch..." << endl;
            return false;
        }

        mesh->initializeMesh();
        prevMesh = mesh;
    }

    meshes[filename] = mesh;

    return true;
}

void TriangleMesh::clearMeshes() {
    meshes.clear();
}

bool TriangleMesh::hasLODs() const
{
    return nextLOD || previousLOD;
}

TriangleMesh *TriangleMesh::getPreviousLOD() {
    return previousLOD ? previousLOD : this;
}

TriangleMesh *TriangleMesh::getNextLOD() {
    return nextLOD ? nextLOD : this;
}

void TriangleMesh::fillOctree(Octree *octree, vector<Octree *> &vertexOctree, unordered_map<int, unordered_set<int>> &vertexToNormalCluster) {
    unordered_map<int, unordered_set<Plane *>> vertexToQuadric;
    int repMode = Application::instance().repMode;
    int clusterMode = Application::instance().clusterMode;

    if (clusterMode == VOXEL_AND_NORMAL) {
        cout << "Prossessing normal clusters..." << endl;
        vertexToQuadric = associateVerticesToQuadrics();
        vertexToNormalCluster = associateVerticesToNormalClusters(vertexToQuadric);
    }
    int nVerts = vertices.size();
    if (repMode == QEM) {
        if (clusterMode != VOXEL_AND_NORMAL)
            vertexToQuadric = associateVerticesToQuadrics();

        // fill and subdivide octree
        vector<unordered_set<Plane *>> octreeToQuadric;
        for (int i = 0; i < nVerts; i++) {
            if (i % (nVerts/20) == 0)
                cout << "[" << round(i/(float)nVerts * 100.f) << "%] Filling octree" << endl;
            vertexOctree.push_back(octree->evaluateVertexQEM(vertices[i], vertexToQuadric, OUT octreeToQuadric, vertexToNormalCluster, minAABB, (maxAABB.x-minAABB.x + 0.2f)/2.f, i));
        }
    } else {
        // fill and subdivide octree
        for (int i = 0; i < nVerts; i++) {
            if (i % (nVerts/20) == 0)
                cout << "[" << round(i/(float)nVerts * 100.f) << "%] Filling octree" << endl;
            vertexOctree.push_back(octree->evaluateVertexAVG(vertices[i], vertexToNormalCluster, minAABB, (maxAABB.x-minAABB.x + 0.2f)/2.f, i));
        }
    }

    // Deallocate used planes
    unordered_set<Plane*> planesToRemove;
    for (pair<int,unordered_set<Plane *>> quadrics : vertexToQuadric) {
        for (Plane* quadric : quadrics.second) {
            planesToRemove.insert(quadric);
        }
    }

    for (Plane* plane : planesToRemove)
        delete plane;
        
    cout << "Done!" << endl;
}

TriangleMesh *TriangleMesh::fillLODs(vector<Octree *> &vertexOctree, unordered_map<int, unordered_set<int>> &vertexToNormalCluster) {
    int clusterMode = Application::instance().clusterMode;

    TriangleMesh *LOD;
    TriangleMesh *prevLOD;
    int diffLODS = max(1, Application::instance().maxLODLevel - Application::instance().minLODLevel);
    for (int lod = 0; lod <= diffLODS; lod++) {
        LOD = new TriangleMesh();
        LOD->LODidx = Application::instance().maxLODLevel - lod;

        if (lod != 0) {
            for (int i = 0; i < vertices.size(); i++)
                vertexOctree[i] = vertexOctree[i]->parent;

            LOD->nextLOD = prevLOD;
            prevLOD->previousLOD = LOD;
        }

        // this will store the index of the new vertices created from the ones inside each octree
        unordered_map<int, int> octreeIdxDict;

        // in case of using normal clustering strategy
        unordered_map<int, vector<int>> octreeIdxClusterDict;
        // new mesh here
        int count = 0;

        // ADD VERTICES
        if (clusterMode == VOXEL_AND_NORMAL) {  // NORMAL CLUSTERING
            for (int i = 0; i < vertices.size(); i++) {
                // add vertex coordinate
                for (int j : vertexToNormalCluster[i]) {
                    if (octreeIdxDict.find(vertexOctree[i]->getIndex() * 8 + j) == octreeIdxDict.end()) {
                        LOD->addVertex(vertexOctree[i]->clusteredRepresentatives[j]);

                        // update dict
                        octreeIdxDict[vertexOctree[i]->getIndex() * 8 + j] = count++;
                    }
                }
            }
        } else {    // STANDARD CLUSTERING
            for (int i = 0; i < vertices.size(); i++) {
                // add vertex coordinate
                if (octreeIdxDict.find(vertexOctree[i]->getIndex()) == octreeIdxDict.end()) {
                    LOD->addVertex(vertexOctree[i]->getPosition());

                    // update dict
                    octreeIdxDict[vertexOctree[i]->getIndex()] = count++;
                }
            }
        }

        std::unordered_map<glm::vec3, bool> facesDict;

        // ADD FACES
        if (clusterMode == VOXEL) {
            for (int i = 0; i < triangles.size(); i += 3) {
                glm::vec3 face = glm::vec3(octreeIdxDict[vertexOctree[triangles[i]]->getIndex()],
                                           octreeIdxDict[vertexOctree[triangles[i + 1]]->getIndex()],
                                           octreeIdxDict[vertexOctree[triangles[i + 2]]->getIndex()]);

                // Face vertices
                if (face.x == face.y || face.x == face.z || face.y == face.z)
                    continue;

                LOD->addTriangle((int)face.x, (int)face.y, (int)face.z);
            }
        } else {
            for (int i = 0; i < triangles.size(); i += 3) {
                glm::vec3 face(0.f);
                bool triangleFound = false;
                // try all possible new faces
                for (auto it1 = vertexToNormalCluster[triangles[i]].begin(); it1 != vertexToNormalCluster[triangles[i]].end(); ++it1) {
                    int clusterIdx_1 = *it1;
                    for (auto it2 = vertexToNormalCluster[triangles[i + 1]].begin(); it2 != vertexToNormalCluster[triangles[i + 1]].end(); ++it2) {
                        int clusterIdx_2 = *it2;
                        for (auto it3 = vertexToNormalCluster[triangles[i + 2]].begin(); it3 != vertexToNormalCluster[triangles[i + 2]].end(); ++it3) {
                            int clusterIdx_3 = *it3;
                            face = glm::vec3(octreeIdxDict[vertexOctree[triangles[i]]->getIndex() * 8 + clusterIdx_1],
                                             octreeIdxDict[vertexOctree[triangles[i + 1]]->getIndex() * 8 + clusterIdx_2],
                                             octreeIdxDict[vertexOctree[triangles[i + 2]]->getIndex() * 8 + clusterIdx_3]);

                            // Face vertices
                            if (face.x == face.y || face.x == face.z || face.y == face.z)
                                continue;

                            triangleFound = true;
                            break;
                        }

                        if (triangleFound) break;
                    }

                    if (triangleFound) break;
                }

                if (facesDict.find(face) != facesDict.end() || facesDict.find(glm::vec3(face.z, face.x, face.y)) != facesDict.end() || facesDict.find(glm::vec3(face.y, face.z, face.x)) != facesDict.end())
                    continue;

                if (triangleFound)
                    LOD->addTriangle((int)face.x, (int)face.y, (int)face.z);

                facesDict[face] = true;
            }
        }
        cout << "Creating LOD " << Application::instance().maxLODLevel - lod << ":" << endl;
        cout << "\tSimplified model faces: " << LOD->triangles.size() / 3 << endl;
        cout << "\tSimplified model vertices: " << LOD->vertices.size() << endl
             << endl;

        LOD->initializeMesh();

        prevLOD = LOD;
    }

    return LOD;
}

// Difference in cost to use next LOD
float TriangleMesh::getCost() const
{
    return cost;
}

// Total cost to use current LOD
float TriangleMesh::getTotalCost() const
{
    return triangles.size()/3;
}
