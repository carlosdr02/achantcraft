#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

static float cameraSpeed = 0.001f;
static float aspectRatio;
static float nearPlane = 0.05f;
static float farPlane = 100.0f;

float& getCameraSpeed() {
    return cameraSpeed;
}

float& getAspectRatio() {
    return aspectRatio;
}

Camera::Camera(const glm::vec3& translation, const glm::vec3& target, float fov) : translation(translation), fov(fov) {
    orientation = glm::normalize(target - translation);
}

glm::vec3 Camera::getRightVector(const glm::vec3& upVector) const {
    return glm::normalize(glm::cross(orientation, upVector));
}

glm::vec3 Camera::getUpVector() const {
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera::getFrontVector(const glm::vec3& rightVector, const glm::vec3& upVector) const {
    return glm::normalize(glm::cross(rightVector, upVector));
}

glm::mat4 Camera::getViewMatrix(const glm::vec3& upVector) const {
    return glm::lookAt(translation, translation + orientation, upVector);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
