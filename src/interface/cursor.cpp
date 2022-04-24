#include "cursor.h"

#include <glm/gtc/matrix_transform.hpp>

static float sens;

void setSensitivity(float sensitivity) {
    sens = sensitivity;
}

static float clamp(float val, float min, float max) {
    if (val < min) {
        return min;
    }

    if (val > max) {
        return max;
    }

    return val;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    static float xLast = xPos;
    static float yLast = yPos;

    static float yaw = -90.0f;
    static float pitch = 0.0f;

    float xOffset = xPos - xLast;
    float yOffset = yLast - yPos; 

    xOffset *= sens;
    yOffset *= sens;

    xLast = xPos;
    yLast = yPos;

    yaw += xOffset;
    pitch += yOffset;

    pitch = clamp(pitch, -89.9f, 89.9f);

    float rYaw = glm::radians(yaw);
    float rPitch = glm::radians(pitch);

    float cPitch = cos(rPitch);

    glm::vec3* orientation = (glm::vec3*)glfwGetWindowUserPointer(window);

    orientation->x = cos(rYaw) * cPitch;
    orientation->y = sin(rPitch);
    orientation->z = sin(rYaw) * cPitch;

    *orientation = glm::normalize(*orientation);
}
