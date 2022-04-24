#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

static float aspect = 16.0f / 9.0f;
static float near = 0.1;
static float far = 100.0f;

Camera::Camera(const glm::vec3& translation, const glm::vec3& target, float fov) : translation(translation), fov(fov) {
    orientation = glm::normalize(target - translation);
}

glm::mat4 Camera::getView() {
    return glm::lookAt(translation, translation + orientation, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjection() {
    return glm::perspective(glm::radians(fov), aspect, near, far);
}
