#include <graphics.h>
#include <camera.h>
#include <cursor.h>
#include <mouse.h>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const char* applicationName = "Achantcraft";

    VkInstance instance = createInstance(applicationName, VK_MAKE_VERSION(1, 0, 0));

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger = createDebugMessenger(instance);
#endif // _DEBUG

    Window window(instance, 1600, 900, applicationName);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Device device(instance, window.surface);

    VkSurfaceCapabilitiesKHR surfaceCapabilities = device.getSurfaceCapabilities(window);
    VkSurfaceFormatKHR surfaceFormat = device.getSurfaceFormat(window.surface);
    VkPresentModeKHR presentMode = device.getSurfacePresentMode(window.surface);
    VkFormat depthFormat = device.getDepthStencilFormat();

    VkRenderPass renderPass = createRenderPass(device.logical, surfaceFormat.format, depthFormat);

    VkQueue graphicsQueue = device.getQueue(0);
    VkQueue presentQueue = device.getQueue(1);

    RendererCreateInfo rendererCreateInfo = {
        .surface             = window.surface,
        .surfaceCapabilities = &surfaceCapabilities,
        .surfaceFormat       = surfaceFormat,
        .presentMode         = presentMode,
        .depthFormat         = depthFormat,
        .renderPass          = renderPass,
        .framesInFlight      = 3,
        .graphicsQueue       = graphicsQueue,
        .presentQueue        = presentQueue
    };

    Renderer renderer(device, rendererCreateInfo);

    VkExtent2D& extent = surfaceCapabilities.currentExtent;

    Scene scene(device);

    renderer.recordCommandBuffers(device.logical, renderPass, extent, scene);

    float& cameraSpeed = getCameraSpeed();

    float& aspectRatio = getAspectRatio();
    aspectRatio = extent.width / (float)extent.height;

    Camera camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 90.0f);

    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glm::vec3 cameraUpVector = camera.getUpVector();
        glm::vec3 cameraRightVector = camera.getRightVector(cameraUpVector);
        glm::vec3 cameraFrontVector = camera.getFrontVector(cameraRightVector, cameraUpVector);

        if (glfwGetKey(window, GLFW_KEY_W)) camera.translation -= cameraFrontVector * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_A)) camera.translation -= cameraRightVector * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S)) camera.translation += cameraFrontVector * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D)) camera.translation += cameraRightVector * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE)) camera.translation += cameraUpVector * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) camera.translation -= cameraUpVector * cameraSpeed;

        glm::mat4 viewProjectionMatrices[] = {
            camera.getViewMatrix(cameraUpVector),
            camera.getProjectionMatrix()
        };

        scene.flushMappedUniformBufferMemory(device.logical, viewProjectionMatrices);

        if (!renderer.draw(device.logical)) {
            int width, height;

            do {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            } while(width == 0 || height == 0);

            surfaceCapabilities = device.getSurfaceCapabilities(window);

            renderer.waitIdle(device.logical);
            renderer.recreate(device, rendererCreateInfo);
            renderer.recordCommandBuffers(device.logical, renderPass, extent, scene);

            aspectRatio = extent.width / (float)extent.height;
        }
    }

    renderer.waitIdle(device.logical);
    scene.destroy(device.logical);
    renderer.destroy(device.logical);

    vkDestroyRenderPass(device.logical, renderPass, nullptr);

    device.destroy();
    window.destroy(instance);

#ifdef _DEBUG
    destroyDebugMessenger(instance, debugMessenger);
#endif // _DEBUG

    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
}
