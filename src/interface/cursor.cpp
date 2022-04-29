#include "cursor.h"

#include "camera.h"

static float sensitivity = 0.1f;

float& getSensitivity() {
    return sensitivity;
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

    static float yaw = 0.0f;
    static float pitch = 0.0f;

    float xOffset = xPos - xLast;
    float yOffset = yLast - yPos; 

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    xLast = xPos;
    yLast = yPos;

    yaw += xOffset;
    pitch += yOffset;

    pitch = clamp(pitch, -89.9f, 89.9f);

    float rYaw = glm::radians(yaw);
    float rPitch = glm::radians(pitch);

    float cPitch = cos(rPitch);

    Camera* camera = (Camera*)glfwGetWindowUserPointer(window);
    glm::vec3& orientation = camera->orientation;

    orientation.x = cos(rYaw) * cPitch;
    orientation.y = sin(rPitch);
    orientation.z = sin(rYaw) * cPitch;
}
