#include "mouse.h"

#include "camera.h"

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Camera* camera = (Camera*)glfwGetWindowUserPointer(window);

    float zoomFactor = 0.1f;

    if (button == GLFW_MOUSE_BUTTON_5) {
        if (action == GLFW_PRESS) {
            camera->fov *= zoomFactor;
        } else if (action == GLFW_RELEASE) {
            camera->fov /= zoomFactor;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
        float& cameraSpeed = getCameraSpeed();

        float velocity = 0.00005f;
        cameraSpeed += yOffset * velocity;

        if (cameraSpeed < velocity) {
            cameraSpeed = velocity;
        }
    }
}
