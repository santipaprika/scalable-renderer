#include "PLYWriter.h"
#include "glm/glm.hpp"
#include <iostream>

using namespace std;

bool PLYWriter::writeMesh(const string &filename, TriangleMesh &mesh) 
{
    ofstream fout;
	int nVertices = mesh.vertices.size();
    int nFaces = mesh.triangles.size() / 3;

	fout.open(filename.c_str(), ios_base::out | ios_base::binary);
	if(!fout.is_open())
		return false;
	if(!writeHeader(fout, nVertices, nFaces))
	{
		fout.close();
		return false;
	}

	writeVertices(fout, nVertices, mesh.vertices);
	writeFaces(fout, nFaces, mesh.triangles);
	fout.close();

    return true;
}

bool PLYWriter::writeHeader(ofstream &fout, int &nVertices, int &nFaces) 
{
    if (nVertices <= 0 || nFaces <= 0)
        return false;

    fout << "ply" << endl;
    fout << "format binary_little_endian 1.0" << endl;
    fout << "element vertex " << nVertices << endl;
    fout << "property float x" << endl;
    fout << "property float y" << endl;
    fout << "property float z" << endl;
    fout << "element face " << nFaces << endl;
    fout << "end_header" << endl;

	return true;
}

void PLYWriter::writeVertices(ofstream &fout, int nVertices, vector<glm::vec3> &vertices) 
{
	for(int i=0; i<nVertices; i++)
	{
		fout.write((char *)&vertices[i].x, sizeof(float));
		fout.write((char *)&vertices[i].y, sizeof(float));
		fout.write((char *)&vertices[i].z, sizeof(float));
	}
}

void PLYWriter::writeFaces(ofstream &fout, int nFaces, vector<int> &triangles) 
{
    unsigned char nVrtxPerFace = 3;
    for(int i=0; i<triangles.size(); i += 3)
	{
		fout.write((char *)&nVrtxPerFace, sizeof(unsigned char));
		fout.write((char *)&triangles[i], sizeof(int));
		fout.write((char *)&triangles[i+1], sizeof(int));
		fout.write((char *)&triangles[i+2], sizeof(int));
	}
}
