#ifndef __PLYWRITER_H__
#define __PLYWRITER_H__

#include <fstream>
#include "TriangleMesh.h"

class PLYWriter {
   public:
    static bool writeMesh(const string &filename, TriangleMesh &mesh);

   private:
    static bool writeHeader(ofstream &fin, int &nVertices, int &nFaces);
    static void writeVertices(ofstream &fin, int nVertices, vector<glm::vec3> &plyVertices);
    static void writeFaces(ofstream &fin, int nFaces, vector<int> &plyTriangles);
};

#endif  // __PLYWRITER_H__