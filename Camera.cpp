#include "Camera.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
#include "Application.h"

#define PI 3.14159f

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::init(glm::vec3 position, float initAngleX, float initAngleY)
{
    distance = position.z;
    angleX = initAngleX;
    angleY = initAngleY;
    near = 0.01f;
    far = 200.0f;
    rangeDistanceCamera[0] = 1.0f;
    rangeDistanceCamera[1] = 3.0f;
    this->position = position;
    velocity = 2;
    sprintVelocity = 20;
    rotationSpeed = 2;

    computeModelViewMatrix();
    computeGridCoordinates();
}

void Camera::resizeCameraViewport(int width, int height)
{
    projection = glm::perspective(60.f / 180.f * PI, float(width) / float(height), near, far);
}

void Camera::rotateCamera(float xRotation, float yRotation)
{
    angleX += xRotation * rotationSpeed;
    angleX = glm::max(-75.0f, glm::min(angleX, 75.0f));
    angleY += yRotation * rotationSpeed;

    computeModelViewMatrix();
}

void Camera::zoomCamera(float distDelta)
{
    distance += distDelta;
    distance = glm::max(rangeDistanceCamera[0], glm::min(distance, rangeDistanceCamera[1]));
    computeModelViewMatrix();
}

void Camera::computeModelViewMatrix()
{
    modelview = glm::mat4(1.0f);
    modelview = glm::rotate(modelview, angleY / 180.f * PI, glm::vec3(0.0f, 1.0f, 0.0f));
    modelview = glm::rotate(modelview, angleX / 180.f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
    modelview[3][0] = position[0];
    modelview[3][1] = position[1];
    modelview[3][2] = position[2];
    modelview_inv = glm::inverse(modelview);
}

void Camera::computeGridCoordinates() 
{
    if (Application::instance().bUpdateVisibility)
        gridPosition = floor(glm::vec2(position.x,position.z) / Application::instance().scene.gridStep + Application::instance().scene.gridSize/2.f);
}

void Camera::move(glm::vec3 delta_direction)
{
    glm::vec3 moveAmount = (-delta_direction * (sprint ? sprintVelocity : velocity));
    glm::vec4 moveAmountH(moveAmount.x, 0.f, moveAmount.z, 1.0f);
    moveAmountH = modelview * moveAmountH;
    moveAmountH.y += moveAmount.y;
    position = glm::vec3(moveAmountH);

    computeModelViewMatrix();
    computeGridCoordinates();
}

glm::mat4 &Camera::getProjectionMatrix()
{
    return projection;
}

glm::mat4 &Camera::getModelViewMatrix()
{
    return modelview;
}

glm::mat4& Camera::getInvModelViewMatrix() 
{
    return modelview_inv;
}

glm::vec3 Camera::getPosition() 
{
    return glm::vec3(modelview[3][0], modelview[3][1], modelview[3][2]);
}

glm::vec2 Camera::getGridCoords() 
{
    return gridPosition;
}