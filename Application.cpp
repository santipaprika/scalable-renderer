#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <iomanip>
#include "Application.h"
#include "ImGUI/imgui.h"


void Application::init()
{
	bPlay = true;
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	scene.init();
	
	for(unsigned int i=0; i<256; i++)
	{
	  keys[i] = false;
	  specialKeys[i] = false;
	}
	mouseButtons[0] = false;
	mouseButtons[1] = false;
	lastMousePos = glm::ivec2(-1, -1);

	frameCount = 0;
	timeCounter = 0;
    framerate = 0;
}

bool Application::loadMesh(const char *filename)
{
  return scene.loadMesh(filename);
}

bool Application::update(int deltaTime)
{
	scene.update(deltaTime);

	frameCount++;
	timeCounter += deltaTime;

	if (timeCounter >= 1000)
	{
		this->framerate = (frameCount * 1000) / float(timeCounter);
		frameCount = 0;
		timeCounter = 0;
	}

	return bPlay;
}

void Application::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.render();
    // std::cout << framerate;
    ImGui::Begin("Framerate");
    ImGui::Text("ImGui FR: %.1f FPS", ImGui::GetIO().Framerate);
    ImGui::Text("Computed FR: %.1f FPS", framerate);
    ImGui::End();
}

void Application::resize(int width, int height)
{
  glViewport(0, 0, width, height);
  scene.getCamera().resizeCameraViewport(width, height);
}

void Application::keyPressed(int key)
{
	if(key == 27) // Escape code
		bPlay = false;
	keys[key] = true;

	if(key > 48 && key < 58)
	  scene.setNumInstances(key - 48);
}

void Application::keyReleased(int key)
{
	keys[key] = false;
}

void Application::specialKeyPressed(int key)
{
	specialKeys[key] = true;
}

void Application::specialKeyReleased(int key)
{
	specialKeys[key] = false;
	if(key == GLUT_KEY_F1)
	  scene.switchPolygonMode();
}

void Application::mouseMove(int x, int y)
{
  // Rotation
	if(mouseButtons[0] && lastMousePos.x != -1)
	  scene.getCamera().rotateCamera(0.5f * (y - lastMousePos.y), 0.5f * (x - lastMousePos.x));

	// Zoom
	if(mouseButtons[1] && lastMousePos.x != -1)
	  scene.getCamera().zoomCamera(0.01f * (y - lastMousePos.y));

 	lastMousePos = glm::ivec2(x, y);
}

void Application::mousePress(int button)
{
  mouseButtons[button] = true;
}

void Application::mouseRelease(int button)
{
  mouseButtons[button] = false;
  if(!mouseButtons[0] && !mouseButtons[1])
    lastMousePos = glm::ivec2(-1, -1);
}

bool Application::getKey(int key) const
{
	return keys[key];
}

bool Application::getSpecialKey(int key) const
{
	return specialKeys[key];
}





