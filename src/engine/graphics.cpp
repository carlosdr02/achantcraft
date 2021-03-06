#include "graphics.h"

#include <stdio.h>

#include <algorithm>
#include <vector>

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

#define VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

static std::vector<const char*> getInstanceExtensions() {
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}

#ifdef _DEBUG
static VkBool32 debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    printf("Debug messenger: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext           = nullptr,
        .flags           = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugMessengerCallback,
        .pUserData       = nullptr
    };

    return debugMessengerCreateInfo;
}

static bool isDebugExtensionAvailable() {
    uint32_t extensionPropertyCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, nullptr);

    VkExtensionProperties* extensionProperties = new VkExtensionProperties[extensionPropertyCount];
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertyCount, extensionProperties);

    bool isExtensionAvailable = false;

    for (uint32_t i = 0; i < extensionPropertyCount; ++i) {
        if (strcmp(extensionProperties[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            isExtensionAvailable = true;
            break;
        }
    }
    
    delete[] extensionProperties;

    return isExtensionAvailable;
}

static bool isValidationLayerAvailable() {
    uint32_t layerPropertyCount;
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);

    VkLayerProperties* layerProperties = new VkLayerProperties[layerPropertyCount];
    vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties);

    bool isLayerAvailable = false;

    for (uint32_t i = 0; i < layerPropertyCount; ++i) {
        if (strcmp(layerProperties[i].layerName, VALIDATION_LAYER_NAME) == 0) {
            isLayerAvailable = true;
            break;
        }
    }
    
    delete[] layerProperties;

    return isLayerAvailable;
}
#endif // _DEBUG

VkInstance createInstance(const char* applicationName, uint32_t applicationVersion) {
    VkApplicationInfo applicationInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = applicationName,
        .applicationVersion = applicationVersion,
        .pEngineName        = "Vortex",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_2
    };

    std::vector<const char*> extensions = getInstanceExtensions();

#ifdef _DEBUG
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo
    };

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = getDebugMessengerCreateInfo();

    if (isDebugExtensionAvailable()) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        instanceCreateInfo.pNext = &debugMessengerCreateInfo;
    } else {
        instanceCreateInfo.pNext = nullptr;
    }

    const char* validationLayer = VALIDATION_LAYER_NAME;

    if (isValidationLayerAvailable()) {
        instanceCreateInfo.enabledLayerCount = 1;
        instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }

    instanceCreateInfo.enabledExtensionCount = extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
#else
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = extensions.size(),
        .ppEnabledExtensionNames = extensions.data()
    };
#endif // _DEBUG

    VkInstance instance;
    vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    return instance;
}

#ifdef _DEBUG
VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
    auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = getDebugMessengerCreateInfo();

    VkDebugUtilsMessengerEXT debugMessenger;

    if (vkCreateDebugUtilsMessengerEXT != nullptr) {
        vkCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger);
    }

    return debugMessenger;
}

void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger) {
    auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
        vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
}
#endif // _DEBUG

Window::Window(VkInstance instance, int width, int height, const char* title) {
    // Create the window.
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    // Create the surface.
    glfwCreateWindowSurface(instance, window, nullptr, &surface);
}

void Window::destroy(VkInstance instance) {
    vkDestroySurfaceKHR(instance, surface, nullptr);

    glfwDestroyWindow(window);
}

Window::operator GLFWwindow*() {
    return window;
}

static std::vector<VkPhysicalDevice> getDiscretePhysicalDevices(VkInstance instance) {
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    auto isNotDiscrete = [](VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        return properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    };

    std::erase_if(physicalDevices, isNotDiscrete);

    return physicalDevices;
}

static VkDeviceSize getPhysicalDeviceMemorySize(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    VkDeviceSize memorySize = 0;

    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
        const VkMemoryHeap& memoryHeap = memoryProperties.memoryHeaps[i];

        if (memoryHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            memorySize += memoryHeap.size;
        }
    }

    return memorySize;
}

Device::Device(VkInstance instance, VkSurfaceKHR surface) {
    // Select a physical device.
    std::vector<VkPhysicalDevice> physicalDevices = getDiscretePhysicalDevices(instance);
    physical = *std::ranges::max_element(physicalDevices, {}, getPhysicalDeviceMemorySize);

    // Select a queue family.
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyPropertyCount, nullptr);

    VkQueueFamilyProperties* queueFamilyProperties = new VkQueueFamilyProperties[queueFamilyPropertyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyPropertyCount, queueFamilyProperties);

    for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamilyProperties[i].queueCount >= 2) {
            VkBool32 surfaceSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &surfaceSupported);

            if (surfaceSupported) {
                queueFamilyIndex = i;
                break;
            }
        }
    }

    delete[] queueFamilyProperties;

    // Create the device.
    float queuePriorities[] = {
        1.0f, 1.0f
    };

    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount       = COUNT_OF(queuePriorities),
        .pQueuePriorities = queuePriorities
    };

    const char* swapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &deviceQueueCreateInfo,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = 1,
        .ppEnabledExtensionNames = &swapchainExtension,
        .pEnabledFeatures        = nullptr
    };

    vkCreateDevice(physical, &deviceCreateInfo, nullptr, &logical);
}

void Device::destroy() {
    vkDestroyDevice(logical, nullptr);
}

VkSurfaceCapabilitiesKHR Device::getSurfaceCapabilities(Window& window) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, window.surface, &surfaceCapabilities);

    uint32_t maxImageCount = surfaceCapabilities.maxImageCount;

    if (maxImageCount == 0) {
        maxImageCount = UINT32_MAX;
    }

    uint32_t& minImageCount = surfaceCapabilities.minImageCount;
    minImageCount = std::clamp(3u, minImageCount, maxImageCount);

    VkExtent2D& extent = surfaceCapabilities.currentExtent;

    if (extent.width == UINT32_MAX) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D minExtent = surfaceCapabilities.minImageExtent;
        VkExtent2D maxExtent = surfaceCapabilities.maxImageExtent;

        extent.width = std::clamp(static_cast<uint32_t>(width), minExtent.width, maxExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(height), minExtent.height, maxExtent.height);
    }

    return surfaceCapabilities;
}

VkSurfaceFormatKHR Device::getSurfaceFormat(VkSurfaceKHR surface) {
    VkFormat formats[] = {
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_SRGB
    };

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &surfaceFormatCount, nullptr);
    
    VkSurfaceFormatKHR* surfaceFormats = new VkSurfaceFormatKHR[surfaceFormatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &surfaceFormatCount, surfaceFormats);

    VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];

    for (uint32_t i = 0; i < COUNT_OF(formats); ++i) {
        for (uint32_t j = 0; j < surfaceFormatCount; ++j) {
            if (surfaceFormats[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && surfaceFormats[j].format == formats[i]) {
                surfaceFormat = surfaceFormats[j];
                goto exit;
            }
        }
    }

exit:
    delete[] surfaceFormats;

    return surfaceFormat;
}

VkPresentModeKHR Device::getSurfacePresentMode(VkSurfaceKHR surface) {
    VkPresentModeKHR presentModes[] = {
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_RELAXED_KHR
    };

    uint32_t surfacePresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &surfacePresentModeCount, nullptr);

    VkPresentModeKHR* surfacePresentModes = new VkPresentModeKHR[surfacePresentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &surfacePresentModeCount, surfacePresentModes);

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < COUNT_OF(presentModes); ++i) {
        for (uint32_t j = 0; j < surfacePresentModeCount; ++j) {
            if (surfacePresentModes[j] == presentModes[i]) {
                presentMode = surfacePresentModes[j];
                goto exit;
            }
        }
    }

exit:
    delete[] surfacePresentModes;

    return presentMode;
}

VkFormat Device::getDepthFormat() {
    VkFormat formats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM
    };

    for (VkFormat format : formats) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physical, format, &properties);

        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkQueue Device::getQueue(uint32_t queueIndex) {
    VkDeviceQueueInfo2 deviceQueueInfo = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
        .pNext            = nullptr,
        .flags            = 0,
        .queueFamilyIndex = queueFamilyIndex,
        .queueIndex       = queueIndex
    };

    VkQueue queue;
    vkGetDeviceQueue2(logical, &deviceQueueInfo, &queue);

    return queue;
}

uint32_t Device::getMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags memoryProperties) {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physical, &physicalDeviceMemoryProperties);

    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        VkMemoryType memoryType = physicalDeviceMemoryProperties.memoryTypes[i];

        if (memoryTypeBits & (1 << i) && (memoryType.propertyFlags & memoryProperties) == memoryProperties) {
            return i;
        }
    }

    return UINT32_MAX;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat) {
    VkAttachmentDescription2 colorAttachmentDescription = {
        .sType          = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .pNext          = nullptr,
        .flags          = 0,
        .format         = colorFormat,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentDescription2 depthAttachmentDescription = {
        .sType          = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .pNext          = nullptr,
        .flags          = 0,
        .format         = depthFormat,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription2 attachmentDescriptions[] = {
        colorAttachmentDescription,
        depthAttachmentDescription
    };

    VkAttachmentReference2 colorAttachmentReference = {
        .sType      = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .pNext      = nullptr,
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .aspectMask = 0
    };

    VkAttachmentReference2 depthAttachmentReference = {
        .sType      = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .pNext      = nullptr,
        .attachment = 1,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .aspectMask = 0
    };

    VkSubpassDescription2 subpassDescription = {
        .sType                   = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pNext                   = nullptr,
        .flags                   = 0,
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .viewMask                = 0,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = nullptr,
        .colorAttachmentCount    = 1,
        .pColorAttachments       = &colorAttachmentReference,
        .pResolveAttachments     = nullptr,
        .pDepthStencilAttachment = &depthAttachmentReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = nullptr
    };

    VkSubpassDependency2 subpassDependency = {
        .sType           = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext           = nullptr,
        .srcSubpass      = VK_SUBPASS_EXTERNAL,
        .dstSubpass      = 0,
        .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask   = 0,
        .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
        .viewOffset      = 0
    };

    VkRenderPassCreateInfo2 renderPassCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .pNext                   = nullptr,
        .flags                   = 0,
        .attachmentCount         = COUNT_OF(attachmentDescriptions),
        .pAttachments            = attachmentDescriptions,
        .subpassCount            = 1,
        .pSubpasses              = &subpassDescription,
        .dependencyCount         = 1,
        .pDependencies           = &subpassDependency,
        .correlatedViewMaskCount = 0,
        .pCorrelatedViewMasks    = nullptr
    };

    VkRenderPass renderPass;
    vkCreateRenderPass2(device, &renderPassCreateInfo, nullptr, &renderPass);

    return renderPass;
}

Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties) {
    // Create the buffer.
    VkBufferCreateInfo bufferCreateInfo = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .size                  = size,
        .usage                 = usage,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr
    };

    vkCreateBuffer(device.logical, &bufferCreateInfo, nullptr, &buffer);

    // Allocate device memory.
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device.logical, buffer, &memoryRequirements);

    uint32_t memoryTypeIndex = device.getMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperties);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext           = nullptr,
        .allocationSize  = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    vkAllocateMemory(device.logical, &memoryAllocateInfo, nullptr, &memory);

    VkBindBufferMemoryInfo bindBufferMemoryInfo = {
        .sType        = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
        .pNext        = nullptr,
        .buffer       = buffer,
        .memory       = memory,
        .memoryOffset = 0
    };

    vkBindBufferMemory2(device.logical, 1, &bindBufferMemoryInfo);
}

void Buffer::destroy(VkDevice device) {
    vkFreeMemory(device, memory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
}

Buffer::operator VkBuffer() {
    return buffer;
}

VkPipelineLayout createPipelineLayout(VkDevice device, uint32_t descriptorSetLayoutCount, const VkDescriptorSetLayout* descriptorSetLayouts) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .setLayoutCount         = descriptorSetLayoutCount,
        .pSetLayouts            = descriptorSetLayouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };

    VkPipelineLayout pipelineLayout;
    vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

    return pipelineLayout;
}

static VkShaderModule createShaderModule(VkDevice device, const char* shaderPath) {
    FILE* file;
    fopen_s(&file, shaderPath, "rb");

    fseek(file, 0L, SEEK_END);
    size_t codeSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    uint32_t* code = new uint32_t[codeSize];
    fread(code, 1, codeSize, file);

    fclose(file);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = codeSize,
        .pCode    = code
    };

    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);

    delete[] code;

    return shaderModule;
}

VkPipeline createComputePipeline(VkDevice device, const char* shaderPath, VkPipelineLayout pipelineLayout) {
    VkShaderModule shaderModule = createShaderModule(device, shaderPath);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
        .module              = shaderModule,
        .pName               = "main",
        .pSpecializationInfo = nullptr
    };

    VkComputePipelineCreateInfo computePipelineCreateInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .stage              = shaderStageCreateInfo,
        .layout             = pipelineLayout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex  = -1
    };

    VkPipeline computePipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &computePipeline);

    vkDestroyShaderModule(device, shaderModule, nullptr);

    return computePipeline;
}

VkPipeline createGraphicsPipeline(VkDevice device, const GraphicsPipelineCreateInfo& createInfo) {
    VkShaderModule vertexShaderModule = createShaderModule(device, createInfo.vertexShaderPath);
    VkShaderModule fragmentShaderModule = createShaderModule(device, createInfo.fragmentShaderPath);

    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_VERTEX_BIT,
        .module              = vertexShaderModule,
        .pName               = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module              = fragmentShaderModule,
        .pName               = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {
        vertexShaderStageCreateInfo,
        fragmentShaderStageCreateInfo
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = nullptr,
        .scissorCount  = 1,
        .pScissors     = nullptr
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = createInfo.polygonMode,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 0.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .front                 = { VK_STENCIL_OP_KEEP },
        .back                  = { VK_STENCIL_OP_KEEP },
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 1.0f
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = 0,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_CLEAR,
        .attachmentCount = 1,
        .pAttachments    = &colorBlendAttachmentState,
        .blendConstants  = { 0.0f }
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = 0,
        .dynamicStateCount = COUNT_OF(dynamicStates),
        .pDynamicStates    = dynamicStates
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stageCount          = COUNT_OF(shaderStageCreateInfos),
        .pStages             = shaderStageCreateInfos,
        .pVertexInputState   = createInfo.vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState   = &multisampleStateCreateInfo,
        .pDepthStencilState  = &depthStencilStateCreateInfo,
        .pColorBlendState    = &colorBlendStateCreateInfo,
        .pDynamicState       = &dynamicStateCreateInfo,
        .layout              = createInfo.pipelineLayout,
        .renderPass          = createInfo.renderPass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1
    };

    VkPipeline graphicsPipeline;
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline);

    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, vertexShaderModule, nullptr);

    return graphicsPipeline;
}

static VkSwapchainKHR createSwapchain(VkDevice device, const RendererCreateInfo& createInfo, VkSwapchainKHR oldSwapchain) {
    const VkSurfaceCapabilitiesKHR* surfaceCapabilities = createInfo.surfaceCapabilities;
    VkSurfaceFormatKHR surfaceFormat = createInfo.surfaceFormat;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = createInfo.surface,
        .minImageCount         = surfaceCapabilities->minImageCount,
        .imageFormat           = surfaceFormat.format,
        .imageColorSpace       = surfaceFormat.colorSpace,
        .imageExtent           = surfaceCapabilities->currentExtent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = surfaceCapabilities->currentTransform,
        .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode           = createInfo.presentMode,
        .clipped               = VK_TRUE,
        .oldSwapchain          = oldSwapchain
    };

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);

    return swapchain;
}

Renderer::Renderer(Device& device, const RendererCreateInfo& createInfo)
        : cameraDataSize(createInfo.cameraDataSize), framesInFlight(createInfo.framesInFlight), frameIndex(0), graphicsQueue(createInfo.graphicsQueue),
        presentQueue(createInfo.presentQueue) {
    // Allocate host memory.
    imageAvailableSemaphores = new VkSemaphore[framesInFlight];
    renderFinishedSemaphores = new VkSemaphore[framesInFlight];
    frameFences = new VkFence[framesInFlight];

    // Create the descriptor set layout.
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
        .binding            = 0,
        .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount    = 1,
        .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = 1,
        .pBindings    = &descriptorSetLayoutBinding
    };

    vkCreateDescriptorSetLayout(device.logical, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

    // Create the command pool.
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .queueFamilyIndex = device.queueFamilyIndex
    };

    vkCreateCommandPool(device.logical, &commandPoolCreateInfo, nullptr, &commandPool);

    // Create the swapchain.
    swapchain = createSwapchain(device.logical, createInfo, VK_NULL_HANDLE);

    // Create the swapchain resources.
    createSwapchainResources(device, createInfo);

    for (uint32_t i = 0; i < framesInFlight; ++i) {
        // Create the semaphores.
        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        vkCreateSemaphore(device.logical, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(device.logical, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);

        // Create the fences.
        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        vkCreateFence(device.logical, &fenceCreateInfo, nullptr, &frameFences[i]);
    }
}

void Renderer::recreate(Device& device, const RendererCreateInfo& createInfo) {
    // Destroy the old swapchain resources.
    destroySwapchainResources(device.logical);

    // Create the new swapchain.
    VkSwapchainKHR newSwapchain = createSwapchain(device.logical, createInfo, swapchain);

    // Destroy the old swapchain.
    vkDestroySwapchainKHR(device.logical, swapchain, nullptr);

    // Store the new swapchain.
    swapchain = newSwapchain;

    // Create the new swapchain resources.
    createSwapchainResources(device, createInfo);
}

void Renderer::destroy(VkDevice device) {
    for (uint32_t i = 0; i < framesInFlight; ++i) {
        vkDestroyFence(device, frameFences[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
    }

    destroySwapchainResources(device);

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    delete[] frameFences;
    delete[] renderFinishedSemaphores;
    delete[] imageAvailableSemaphores;
}

void Renderer::recordCommandBuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent) {
    vkResetCommandPool(device, commandPool, 0);

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .pInheritanceInfo = nullptr
        };

        vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

        VkRect2D renderArea = {
            .offset = { 0, 0 },
            .extent = extent
        };

        VkClearValue clearValues[] = {
            { 0.0f, 0.0f, 0.0f, 1.0f },
            { 1.0f, 0 }
        };

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext           = nullptr,
            .renderPass      = renderPass,
            .framebuffer     = framebuffers[i],
            .renderArea      = renderArea,
            .clearValueCount = COUNT_OF(clearValues),
            .pClearValues    = clearValues
        };

        VkSubpassBeginInfo subpassBeginInfo = {
            .sType    = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
            .pNext    = nullptr,
            .contents = VK_SUBPASS_CONTENTS_INLINE
        };

        vkCmdBeginRenderPass2(commandBuffers[i], &renderPassBeginInfo, &subpassBeginInfo);

        VkSubpassEndInfo subpassEndInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
            .pNext = nullptr
        };

        vkCmdEndRenderPass2(commandBuffers[i], &subpassEndInfo);

        vkEndCommandBuffer(commandBuffers[i]);
    }
}

bool Renderer::draw(VkDevice device, const void* cameraData) {
    if (vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex) == VK_ERROR_OUT_OF_DATE_KHR) {
        return false;
    }

    if (imageFences[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imageFences[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imageFences[imageIndex] = frameFences[frameIndex];

    vkWaitForFences(device, 1, &frameFences[frameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frameFences[frameIndex]);

    VkDeviceSize offset = imageIndex * cameraDataSize;
    memcpy(static_cast<char*>(mappedUniformBufferMemory) + offset, cameraData, cameraDataSize);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &imageAvailableSemaphores[frameIndex],
        .pWaitDstStageMask    = &waitStage,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &commandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &renderFinishedSemaphores[frameIndex]
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameFences[frameIndex]);

    VkPresentInfoKHR presentInfo = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &renderFinishedSemaphores[frameIndex],
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain,
        .pImageIndices      = &imageIndex,
        .pResults           = nullptr
    };

    vkQueuePresentKHR(presentQueue, &presentInfo);

    frameIndex = (frameIndex + 1) % framesInFlight;

    return true;
}

void Renderer::waitIdle(VkDevice device) {
    vkWaitForFences(device, framesInFlight, frameFences, VK_TRUE, UINT64_MAX);
}

void Renderer::createSwapchainResources(Device& device, const RendererCreateInfo& createInfo) {
    // Get the swapchain image count.
    vkGetSwapchainImagesKHR(device.logical, swapchain, &swapchainImageCount, nullptr);

    // Allocate host memory.
    swapchainImages = new VkImage[swapchainImageCount];
    depthImages = new VkImage[swapchainImageCount];
    swapchainImageViews = new VkImageView[swapchainImageCount];
    depthImageViews = new VkImageView[swapchainImageCount];
    framebuffers = new VkFramebuffer[swapchainImageCount];
    descriptorSets = new VkDescriptorSet[swapchainImageCount];
    commandBuffers = new VkCommandBuffer[swapchainImageCount];
    imageFences = new VkFence[swapchainImageCount]();

    // Get the swapchain images.
    vkGetSwapchainImagesKHR(device.logical, swapchain, &swapchainImageCount, swapchainImages);

    // Create the depth images.
    VkFormat depthFormat = createInfo.depthFormat;
    VkExtent2D extent = createInfo.surfaceCapabilities->currentExtent;

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkImageCreateInfo imageCreateInfo = {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .imageType             = VK_IMAGE_TYPE_2D,
            .format                = depthFormat,
            .extent                = { extent.width, extent.height, 1 },
            .mipLevels             = 1,
            .arrayLayers           = 1,
            .samples               = VK_SAMPLE_COUNT_1_BIT,
            .tiling                = VK_IMAGE_TILING_OPTIMAL,
            .usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
        };

        vkCreateImage(device.logical, &imageCreateInfo, nullptr, &depthImages[i]);
    }

    // Allocate device memory.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device.logical, depthImages[0], &memoryRequirements);

    uint32_t memoryTypeIndex = device.getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext           = nullptr,
        .allocationSize  = swapchainImageCount * memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    vkAllocateMemory(device.logical, &memoryAllocateInfo, nullptr, &depthImagesMemory);

    VkBindImageMemoryInfo* bindImageMemoryInfos = new VkBindImageMemoryInfo[swapchainImageCount];

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkBindImageMemoryInfo bindImageMemoryInfo = {
            .sType        = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
            .pNext        = nullptr,
            .image        = depthImages[i],
            .memory       = depthImagesMemory,
            .memoryOffset = i * memoryRequirements.size
        };

        bindImageMemoryInfos[i] = bindImageMemoryInfo;
    }

    vkBindImageMemory2(device.logical, swapchainImageCount, bindImageMemoryInfos);

    delete[] bindImageMemoryInfos;

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        // Create the swapchain image views.
        VkImageSubresourceRange imageSubresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        };

        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .image            = swapchainImages[i],
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = createInfo.surfaceFormat.format,
            .components       = { VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = imageSubresourceRange
        };

        vkCreateImageView(device.logical, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);

        // Create the depth image views.
        imageViewCreateInfo.image = depthImages[i];
        imageViewCreateInfo.format = depthFormat;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        vkCreateImageView(device.logical, &imageViewCreateInfo, nullptr, &depthImageViews[i]);

        // Create the framebuffers.
        VkImageView attachments[] = {
            swapchainImageViews[i],
            depthImageViews[i]
        };

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = createInfo.renderPass,
            .attachmentCount = COUNT_OF(attachments),
            .pAttachments    = attachments,
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1
        };

        vkCreateFramebuffer(device.logical, &framebufferCreateInfo, nullptr, &framebuffers[i]);
    }

    // Create the uniform buffer.
    VkDeviceSize uniformBufferSize = swapchainImageCount * cameraDataSize;
    uniformBuffer = Buffer(device, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Map the uniform buffer memory.
    vkMapMemory(device.logical, uniformBuffer.memory, 0, uniformBufferSize, 0, &mappedUniformBufferMemory);

    // Create the descriptor pool.
    VkDescriptorPoolSize descriptorPoolSize = {
        .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = swapchainImageCount
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .maxSets       = swapchainImageCount,
        .poolSizeCount = 1,
        .pPoolSizes    = &descriptorPoolSize
    };

    vkCreateDescriptorPool(device.logical, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

    // Allocate the descriptor sets.
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(swapchainImageCount, descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = descriptorPool,
        .descriptorSetCount = swapchainImageCount,
        .pSetLayouts        = descriptorSetLayouts.data()
    };

    vkAllocateDescriptorSets(device.logical, &descriptorSetAllocateInfo, descriptorSets);

    // Update the descriptor sets.
    VkDescriptorBufferInfo* descriptorBufferInfos = new VkDescriptorBufferInfo[swapchainImageCount];
    VkWriteDescriptorSet* writeDescriptorSets = new VkWriteDescriptorSet[swapchainImageCount];

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        VkDescriptorBufferInfo descriptorBufferInfo = {
            .buffer = uniformBuffer,
            .offset = i * cameraDataSize,
            .range  = cameraDataSize
        };

        descriptorBufferInfos[i] = descriptorBufferInfo;

        VkWriteDescriptorSet writeDescriptorSet = {
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptorSets[i],
            .dstBinding       = 0,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &descriptorBufferInfos[i],
            .pTexelBufferView = nullptr
        };

        writeDescriptorSets[i] = writeDescriptorSet;
    }

    vkUpdateDescriptorSets(device.logical, swapchainImageCount, writeDescriptorSets, 0, nullptr);

    delete[] writeDescriptorSets;
    delete[] descriptorBufferInfos;

    // Allocate the command buffers.
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = swapchainImageCount
    };

    vkAllocateCommandBuffers(device.logical, &commandBufferAllocateInfo, commandBuffers);
}

void Renderer::destroySwapchainResources(VkDevice device) {
    vkFreeCommandBuffers(device, commandPool, swapchainImageCount, commandBuffers);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    uniformBuffer.destroy(device);

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, depthImageViews[i], nullptr);
        vkDestroyImageView(device, swapchainImageViews[i], nullptr);
    }

    vkFreeMemory(device, depthImagesMemory, nullptr);

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        vkDestroyImage(device, depthImages[i], nullptr);
    }

    delete[] imageFences;
    delete[] commandBuffers;
    delete[] descriptorSets;
    delete[] framebuffers;
    delete[] depthImageViews;
    delete[] swapchainImageViews;
    delete[] depthImages;
    delete[] swapchainImages;
}
