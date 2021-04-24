#include <iostream>
#include <vector>
#include <set>
#include "TriangleMesh.h"
#include "PLYReader.h"
#include "Scene.h"
#include "Application.h"

using namespace std;

TriangleMesh::TriangleMesh()
{
}

void TriangleMesh::addVertex(const glm::vec3 &position)
{
    vertices.push_back(position);
}

void TriangleMesh::addTriangle(int v0, int v1, int v2)
{
    triangles.push_back(v0);
    triangles.push_back(v1);
    triangles.push_back(v2);
}

void TriangleMesh::initializeMesh() 
{
    computeAABB();
    sendToOpenGL(Application::instance().scene.basicProgram);
}

void TriangleMesh::buildCube()
{
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

void TriangleMesh::sendToOpenGL(ShaderProgram &program)
{
    vector<float> data;

    for (unsigned int tri = 0; tri < triangles.size(); tri += 3)
    {
        glm::vec3 normal;

        normal = glm::cross(vertices[triangles[tri + 1]] - vertices[triangles[tri]],
                            vertices[triangles[tri + 2]] - vertices[triangles[tri]]);
        normal = glm::normalize(normal);
        for (unsigned int vrtx = 0; vrtx < 3; vrtx++)
        {
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

void TriangleMesh::render() const
{
    glBindVertexArray(vao);
    glEnableVertexAttribArray(posLocation);
    glEnableVertexAttribArray(normalLocation);
    glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 3 * triangles.size() / 3);
}

void TriangleMesh::free()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    vertices.clear();
    triangles.clear();
}

void TriangleMesh::computeAABB()
{
    minAABB = glm::vec3(INFINITY);
    maxAABB = glm::vec3(-INFINITY);
    for (int i = 0; i < vertices.size(); i++)
    {
        minAABB.x = min(minAABB.x, vertices[i].x);
        minAABB.y = min(minAABB.y, vertices[i].y);
        minAABB.z = min(minAABB.z, vertices[i].z);

        maxAABB.x = max(maxAABB.x, vertices[i].x);
        maxAABB.y = max(maxAABB.y, vertices[i].y);
        maxAABB.z = max(maxAABB.z, vertices[i].z);
    }
}

glm::vec3 TriangleMesh::getExtents() const
{
    return maxAABB - minAABB;
}

TriangleMesh* TriangleMesh::computeLODs(Octree *octree)
{
    Octree *vertexOctree[vertices.size()];

    for (int i=0; i<vertices.size(); i++)
    {
        vertexOctree[i] = octree->evaluateVertex(vertices[i]);
    }

    // this will store the index of the new vertices created from the ones inside each octree
    std::unordered_map<Octree*, int> octreeIdxDict;

    TriangleMesh* LOD = new TriangleMesh();

    int count = 0;
    for (int i=0; i <vertices.size(); i++)
    {
        // Octree idx is not present
        if (octreeIdxDict.find(vertexOctree[i]) == octreeIdxDict.end())
        {
            // add vertex coordinate
            LOD->addVertex(vertexOctree[i]->getAvgPosition());

            // store the index of the new vertex inserted above
            octreeIdxDict[vertexOctree[i]] = count;
            count++;
        }

    }
    std::cout << count << std::endl;

    std::vector<glm::vec3> faces;
    for (int i = 0; i < LOD->vertices.size(); i += 3)
    {
        glm::vec3 face = glm::vec3(octreeIdxDict[vertexOctree[i]], octreeIdxDict[vertexOctree[i+1]], octreeIdxDict[vertexOctree[i+2]]);

        // Face vertices
        if (face.x == face.y || face.x == face.z || face.y == face.z)
            continue;

        bool faceExists = false;
        for (glm::vec3 existingFace : faces)
        {
            // those will be integer indices, so we are safe at epsilon = 0.001
            if ((face - existingFace).length() < 0.001f || ((glm::vec3(face.z, face.x, face.y)-existingFace).length() < 0.001f) || ((glm::vec3(face.y, face.z, face.x)-existingFace).length() < 0.001f))
                faceExists = true;
        }

        if (faceExists)
            continue;

        LOD->addTriangle(face.x, face.y, face.z);
        faces.push_back(face);
    }

    delete octree;

    
    for (glm::vec3 face : faces)
    {
        LOD->addTriangle((int)face.x, (int)face.y, (int)face.z);
    }

    LOD->initializeMesh();

    return LOD;
}

unordered_map<string, TriangleMesh *> TriangleMesh::meshes = {};
TriangleMesh *TriangleMesh::Get(string filename)
{
    if (meshes.find(filename) != meshes.end())
        return meshes.at(filename);

    TriangleMesh *mesh = new TriangleMesh();
    bool bSuccess = PLYReader::readMesh(filename, (*mesh));
    if (bSuccess)
    {
        mesh->initializeMesh();
        meshes[filename] = mesh;
        Octree *octree = new Octree(6, mesh->minAABB - glm::vec3(0.001f), mesh->maxAABB + glm::vec3(0.001f));
        meshes[filename] = mesh->computeLODs(octree);
    }

    return mesh;
}
