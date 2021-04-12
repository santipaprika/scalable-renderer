#ifndef _CAMERA_INCLUDE
#define _CAMERA_INCLUDE

#include <glm/glm.hpp>

// Camera contains the properies of the camera the scene is using
// It is responsible for computing the associated GL matrices

class Camera
{

public:
    Camera();
    ~Camera();

    void init(glm::vec3 position, float initAngleX = 0.0f, float initAngleY = 0.0f);

    void resizeCameraViewport(int width, int height);
    void rotateCamera(float xRotation, float yRotation);
    void zoomCamera(float distDelta);
    void move(glm::vec3 delta_direction);

    glm::mat4 &getProjectionMatrix();
    glm::mat4 &getModelViewMatrix();

    bool sprint;

private:
    void computeModelViewMatrix();

private:
    float angleX, angleY, distance; // Camera parameters
    glm::vec3 position;

    float velocity;
    float sprintVelocity;

    float rotationSpeed;
    float rangeDistanceCamera[2];

    float near;
    float far;
    
    glm::mat4 projection, modelview; // OpenGL matrices
};

#endif // _CAMERA_INCLUDE
