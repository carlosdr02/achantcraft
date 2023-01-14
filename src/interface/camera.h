#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 translation;
    glm::vec3 orientation;
    float fov = 90.0f;
    float sensitivity = 0.1f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    void orientate(float xPos, float yPos);
};