#include "Camera.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

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
    rangeDistanceCamera[0] = 1.0f;
    rangeDistanceCamera[1] = 3.0f;
    this->position = position;
    velocity = 2;
    rotationSpeed = 5;

    computeModelViewMatrix();
}

void Camera::resizeCameraViewport(int width, int height)
{
    projection = glm::perspective(60.f / 180.f * PI, float(width) / float(height), 0.01f, 100.0f);
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
    //modelview = glm::lookAt(position, position + glm::vec3(sin(angleY),0,cos(angleY)), glm::vec3(0,1,0));
    modelview[3][0] = position[0];
    modelview[3][1] = position[1];
    modelview[3][2] = position[2];
    modelview = glm::inverse(modelview);

}

void Camera::move(glm::vec3 delta_direction)
{
    glm::vec3 moveAmount = (-delta_direction * velocity);
    glm::vec4 moveAmountH(moveAmount, 1.0f);
    moveAmountH = glm::inverse(modelview) * moveAmountH;
    position = glm::vec3(moveAmountH);

    // if (delta_direction.y != 0)
    //     position = glm::vec3(0, moveAmountH.y, 0);
    // else
    //     position = glm::vec3(moveAmountH.x, 0, moveAmountH.z);


    computeModelViewMatrix();
}

glm::mat4 &Camera::getProjectionMatrix()
{
    return projection;
}

glm::mat4 &Camera::getModelViewMatrix()
{
    return modelview;
}