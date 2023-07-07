#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VkInstance createInstance();

class Queue {
public:
    uint32_t familyIndex;

    operator VkQueue();
    VkQueue* operator&();

private:
    VkQueue queue;
};

class Device {
public:
    VkPhysicalDevice physical;
    Queue renderQueue;
    VkDevice logical;

    Device() = default;
    Device(VkInstance instance, VkSurfaceKHR surface);
    void destroy();

    VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface, GLFWwindow* window);
    VkSurfaceFormatKHR getSurfaceFormat(VkSurfaceKHR surface);

    uint32_t getMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperties);
};

class Buffer {
public:
    VkDeviceMemory memory;

    Buffer() = default;
    Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
    void destroy(VkDevice device);

    VkDeviceAddress getDeviceAddress(VkDevice device);

    operator VkBuffer();

private:
    VkBuffer buffer;
};

struct RendererCreateInfo {
    VkSurfaceKHR surface;
    const VkSurfaceCapabilitiesKHR* surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormat;
    uint32_t framesInFlight;
};

class Renderer {
public:
    Renderer() = default;
    Renderer(Device& device, const RendererCreateInfo& createInfo);
    void destroy(VkDevice device);

    void resize(VkDevice device, const RendererCreateInfo& createInfo);

private:
    VkSwapchainKHR swapchain;

    void createSwapchain(VkDevice device, const RendererCreateInfo& createInfo, VkSwapchainKHR oldSwapchain);
};
