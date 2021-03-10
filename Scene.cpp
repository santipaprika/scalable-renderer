#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"


Scene::Scene()
{
	mesh = NULL;
}

Scene::~Scene()
{
	if(mesh != NULL)
		delete mesh;
}


void Scene::init()
{
	initShaders();
	mesh = new TriangleMesh();
	mesh->buildCube();
	mesh->sendToOpenGL(basicProgram);
	currentTime = 0.0f;
	
	camera.init(2.0f);
	
	bPolygonFill = true;

	meshInstances_dim1 = 1;
}

bool Scene::loadMesh(const char *filename)
{
	PLYReader reader;

	mesh->free();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if(bSuccess)
		mesh->computeAABB();
	  mesh->sendToOpenGL(basicProgram);
	
	return bSuccess;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
}

void Scene::render()
{
	glm::mat3 normalMatrix;

	float x_step = mesh->getExtents().r * 5 / 4;
	float y_step = mesh->getExtents().g * 5 / 4;

	for (float i=x_step/2*(-meshInstances_dim1 + 1); i <= (meshInstances_dim1 + x_step)/2; i += x_step)
	{
		for (float j=y_step/2*(-meshInstances_dim1 + 1); j <= (meshInstances_dim1 + y_step)/2; j += y_step)
		{
			basicProgram.use();
			basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());

			glm::mat4 modelViewMatrix = glm::translate(camera.getModelViewMatrix(), glm::vec3(i, j, 0));

			basicProgram.setUniformMatrix4f("modelview", modelViewMatrix);
			normalMatrix = glm::inverseTranspose(modelViewMatrix);
			basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
			
			basicProgram.setUniform1i("bLighting", bPolygonFill?1:0);
			if(bPolygonFill)
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
					mesh->render();
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDisable(GL_POLYGON_OFFSET_FILL);
				basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
			}
			mesh->render();
		}
	}
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
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



