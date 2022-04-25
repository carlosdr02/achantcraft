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

    VkDeviceSize vertexBufferSize = 32 * 32 * 16;
    Scene scene(device, renderPass, vertexBufferSize);

    // PERLIN NOISE

    // Create the descriptor pool.
    VkDescriptorPoolSize descriptorPoolSize = {
        .type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .maxSets       = 1,
        .poolSizeCount = 1,
        .pPoolSizes    = &descriptorPoolSize
    };

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device.logical, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

    // Create the descriptor set layout.
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
        .binding            = 0,
        .descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount    = 1,
        .stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = 1,
        .pBindings    = &descriptorSetLayoutBinding
    };

    VkDescriptorSetLayout descriptorSetLayout;
    vkCreateDescriptorSetLayout(device.logical, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

    // Allocate the descriptor set.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &descriptorSetLayout
    };

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device.logical, &descriptorSetAllocateInfo, &descriptorSet);

    // Update the descriptor set.
    VkDescriptorBufferInfo descriptorBufferInfo = {
        .buffer = *scene.vertexBuffer,
        .offset = 0,
        .range  = vertexBufferSize
    };

    VkWriteDescriptorSet writeDescriptorSet = {
        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = descriptorSet,
        .dstBinding       = 0,
        .dstArrayElement  = 0,
        .descriptorCount  = 1,
        .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo       = nullptr,
        .pBufferInfo      = &descriptorBufferInfo,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets(device.logical, 1, &writeDescriptorSet, 0, nullptr);

    // Create the pipeline layout.
    VkPipelineLayout pipelineLayout = createPipelineLayout(device.logical, 1, &descriptorSetLayout);

    // Create the compute pipeline.
    VkPipeline computePipeline = createComputePipeline(device.logical, "../src/game/noise.spv", pipelineLayout);

    // Create the command pool.
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = device.queueFamilyIndex
    };

    VkCommandPool commandPool;
    vkCreateCommandPool(device.logical, &commandPoolCreateInfo, nullptr, &commandPool);

    // Allocate the command buffer.
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device.logical, &commandBufferAllocateInfo, &commandBuffer);

    // Record the command buffer.
    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdDispatch(commandBuffer, 1, 1, 1);

    vkEndCommandBuffer(commandBuffer);

    // Submit the command buffer.
    VkSubmitInfo submitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 0,
        .pWaitSemaphores      = nullptr,
        .pWaitDstStageMask    = nullptr,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores    = nullptr
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    // END PERLIN NOISE

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

    // PERLIN NOISE
    vkDestroyCommandPool(device.logical, commandPool, nullptr);
    vkDestroyPipeline(device.logical, computePipeline, nullptr);
    vkDestroyPipelineLayout(device.logical, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.logical, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device.logical, descriptorPool, nullptr);
    // END PERLIN NOISE

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
