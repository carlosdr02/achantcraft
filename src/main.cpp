#include <graphics.h>
#include <camera.h>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const char* applicationName = "Achantcraft";

    VkInstance instance = createInstance(applicationName, VK_MAKE_VERSION(1, 0, 0));

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger = createDebugMessenger(instance);
#endif // _DEBUG

    Window window(instance, 1600, 900, applicationName);

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

    float vertices[] = {
        0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    VkDeviceSize vertexBufferSize = sizeof(vertices);

    Scene scene(device, renderPass, vertexBufferSize);

    void* mappedVertexBufferMemory;
    vkMapMemory(device.logical, scene.vertexBuffer->memory, 0, vertexBufferSize, 0, &mappedVertexBufferMemory);
    memcpy(mappedVertexBufferMemory, vertices, vertexBufferSize);
    vkUnmapMemory(device.logical, scene.vertexBuffer->memory);

    renderer.recordCommandBuffers(device.logical, renderPass, surfaceCapabilities.currentExtent, scene);

    Camera camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 90.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_W)) camera.translation.z -= 0.001f;
        if (glfwGetKey(window, GLFW_KEY_A)) camera.translation.x -= 0.001f;
        if (glfwGetKey(window, GLFW_KEY_S)) camera.translation.z += 0.001f;
        if (glfwGetKey(window, GLFW_KEY_D)) camera.translation.x += 0.001f;

        glm::mat4 viewProjectionMatrices[] = {
            camera.getView(),
            camera.getProjection()
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
            renderer.recordCommandBuffers(device.logical, renderPass, surfaceCapabilities.currentExtent, scene);
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
