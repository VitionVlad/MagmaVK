#include <iostream>

#include <vector>

#include <array>

#include <string>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/transform.hpp>

#include "Reader.hpp"

using namespace std;

using namespace glm;

class MagmaVK{
    public:
    GLFWwindow* window;
    ivec2 resolution = {1280, 720};
    ivec2 oldresolution = {1280, 720};
    vec2 rot;
    vec3 pos;
    VkInstance instance;
    VkPhysicalDevice physdevice;
    VkDevice device;
    VkSurfaceKHR Surface;
    VkSwapchainKHR swapchain;
    VkExtent2D ext2d;
    VkPhysicalDeviceProperties physprop;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkImage> swapChainImages;
    VkViewport viewport;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline pipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    mat4 proj;
    mat4 view;
    mat4 MVP;
    const int prerenderframes = 2;
    uint32_t currentFrame = 0;
    const vec3 vertexpos[3] = {
        vec3(0.0, -0.5, 0),
        vec3(0.5, 0.5, 0),
        vec3(-0.5, 0.5, 0)
    };
    VkBuffer vertexBuffer;
    VkVertexInputBindingDescription bindingDescription{};
    VkVertexInputAttributeDescription attrdesc;
    VkDeviceMemory vertexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    string fshaderpath = "Engine/raw/surfacefrag.spv";
    string vshaderpath = "Engine/raw/surfacevert.spv";
    void CreateInstance(){
        VkApplicationInfo appinfo;
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.apiVersion = VK_API_VERSION_1_0;
        appinfo.pNext = nullptr;
        appinfo.pApplicationName = "NuclearTech";
        appinfo.pEngineName = "NuclearTechVK";
        appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        VkInstanceCreateInfo instinfo;
        instinfo.pApplicationInfo = &appinfo;
        instinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        uint32 extcount;
        const char** ext = glfwGetRequiredInstanceExtensions(&extcount);
        instinfo.ppEnabledExtensionNames = ext;
        instinfo.enabledExtensionCount = extcount;
        VkLayerProperties lprop;
        uint32 layercont;
        vkEnumerateInstanceLayerProperties(&layercont, &lprop);
        const char* lrname = lprop.layerName;
        instinfo.ppEnabledLayerNames = &lrname;
        instinfo.enabledLayerCount = layercont;
        vkCreateInstance(&instinfo, nullptr, &instance);
        cout << "Log:instance created" << endl;
    }
    void CreateDevice(){
        uint32 devicecount;
        vkEnumeratePhysicalDevices(instance, &devicecount, nullptr);
        vkEnumeratePhysicalDevices(instance, &devicecount, &physdevice);
        VkLayerProperties lprop;
        uint32 layercont;
        vkEnumerateDeviceLayerProperties(physdevice, &layercont, &lprop);
        vkGetPhysicalDeviceProperties(physdevice, &physprop);
        cout << "Device Name: " << physprop.deviceName << endl;
        cout << "Device Type: " << physprop.deviceType << endl;
        cout << "Api version: " << physprop.apiVersion << endl;
        
        VkDeviceQueueCreateInfo queueinfo = {};
        const float prior[1] = {1.0f};
       queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
       queueinfo.pNext = NULL;
       queueinfo.queueCount = 1;
       queueinfo.pQueuePriorities = prior;

        const char* extname = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        VkDeviceCreateInfo deviceinfo = {};
        deviceinfo.flags = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceinfo.enabledExtensionCount = 1;
        deviceinfo.ppEnabledExtensionNames = &extname;
        deviceinfo.enabledLayerCount = layercont;
        const char* lrname = lprop.layerName;
        deviceinfo.ppEnabledLayerNames = &lrname;
        deviceinfo.queueCreateInfoCount = 1;
        deviceinfo.pQueueCreateInfos = &queueinfo;
        vkCreateDevice(physdevice, &deviceinfo, nullptr, &device);
        vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &presentQueue);
        cout << "Log:Device created" << endl;
    }
    void CreateSwapChain(bool dbginfo){
        VkSurfaceCapabilitiesKHR capap;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physdevice, Surface, &capap);
        uint32 surfcont;
        VkSurfaceFormatKHR form;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physdevice, Surface, &surfcont, &form);
        uint32_t imageCount = capap.minImageCount + 1;
        VkSwapchainCreateInfoKHR swapinfo = {};
        swapinfo.clipped = 1;
        swapinfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        swapinfo.imageArrayLayers = capap.maxImageArrayLayers;
        swapinfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapinfo.imageExtent = capap.currentExtent;
        swapinfo.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        swapinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapinfo.minImageCount = capap.minImageCount;
        swapinfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapinfo.surface = Surface;
        swapinfo.oldSwapchain = VK_NULL_HANDLE;
        if(dbginfo == 1){
            cout << "Log:SwapChainCreateInfo Filled" << endl;
        }
        vkCreateSwapchainKHR(device, &swapinfo, nullptr, &swapchain);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data());
        if(dbginfo == 1){
            cout << "Log:SwapChain Created" << endl;
        }
    }
    void CreateImageViews(bool dbginfo){
        swapChainImageViews.resize(swapChainImages.size());
        for(size_t i = 0; i < swapChainImages.size(); i++){
            VkImageViewCreateInfo imginfo = {};
            imginfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imginfo.image = swapChainImages[i];
            imginfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imginfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imginfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imginfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imginfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imginfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imginfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imginfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imginfo.subresourceRange.baseMipLevel = 0;
            imginfo.subresourceRange.levelCount = 1;
            imginfo.subresourceRange.baseArrayLayer = 0;
            imginfo.subresourceRange.layerCount = 1;
            vkCreateImageView(device, &imginfo, nullptr, &swapChainImageViews[i]);
            if(dbginfo == 1){
                cout << "Log:Image View " << i << " Created" << endl;
            }
        }
    };
    VkShaderModule shadermodule(const std::vector<char>& code){
        VkShaderModuleCreateInfo shadrinfo;
        shadrinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shadrinfo.codeSize = code.size();
        shadrinfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderm;
        vkCreateShaderModule(device, &shadrinfo, nullptr, &shaderm);
        return shaderm;
    }
    void CreatePipeline(bool dbginfo){
       auto vertShaderCode = loadbin(vshaderpath);
        auto fragShaderCode = loadbin(fshaderpath);
        VkShaderModule vertShaderModule = shadermodule(vertShaderCode);
        VkShaderModule fragShaderModule = shadermodule(fragShaderCode);
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = &attrdesc;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) resolution.x;
        viewport.height = (float) resolution.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent.width = (float) resolution.x;
        scissor.extent.height = (float) resolution.y;
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        if(dbginfo == 1){
            cout << "Log:Pipeline Layout Created" << endl;
        }
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        if(dbginfo == 1){
            cout << "Log:Pipeline Created" << endl;
        }
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }
    void CreateRenderPass(bool dbginfo){
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription colatach = {};
        colatach.format = VK_FORMAT_R8G8B8A8_SRGB;
        colatach.samples = VK_SAMPLE_COUNT_1_BIT;
        colatach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colatach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colatach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colatach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colatach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colatach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        std::array<VkAttachmentDescription, 2> attachments = {colatach, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
        if(dbginfo == 1){
            cout << "Log:Render Pass Created" << endl;
        }
    }
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateImage(device, &imageInfo, nullptr, &image);
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physdevice, &memProperties);
        uint32 finmem;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (memRequirements.memoryTypeBits & (1 << i)) {
                finmem = i;
                break;
            }
        }
        allocInfo.memoryTypeIndex = finmem;
        vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
        vkBindImageMemory(device, image, imageMemory, 0);
    }
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VkImageView imageView;
        vkCreateImageView(device, &viewInfo, nullptr, &imageView);
        return imageView;
    }
    void CreateDepthRes(){
        createImage(resolution.x, resolution.y, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    void CreateVertexInput(){
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(vec3);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        attrdesc.binding = 0;
        attrdesc.location = 0;
        attrdesc.format = VK_FORMAT_R32G32B32_SFLOAT;
        attrdesc.offset = 0;
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(vertexpos);
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physdevice, &memProperties);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        uint32 finmem;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (memRequirements.memoryTypeBits & (1 << i)) {
                finmem = i;
                break;
            }
        }
        allocInfo.memoryTypeIndex = finmem;
        vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory);
        vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
        void* data;
        vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
        memcpy(data, vertexpos, (size_t) bufferInfo.size);
        vkUnmapMemory(device, vertexBufferMemory);
    }
    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    }
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physdevice, &memProperties);
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        uint32 finmem;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (memRequirements.memoryTypeBits & (1 << i)) {
                finmem = i;
                break;
            }
        }
        allocInfo.memoryTypeIndex = finmem;
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }
    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(MVP);

        uniformBuffers.resize(prerenderframes);
        uniformBuffersMemory.resize(prerenderframes);

        for (size_t i = 0; i < prerenderframes; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        }
    }
    void CreateDescriptoPool(){
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(prerenderframes);
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(prerenderframes);
        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    }
    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(prerenderframes, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(prerenderframes);
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(prerenderframes);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        for (size_t i = 0; i < prerenderframes; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MVP);
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }
    void CreateFramebuffer(bool dbginfo){
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = resolution.x;
            framebufferInfo.height = resolution.y;
            framebufferInfo.layers = 1;
            vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);
            if(dbginfo == 1){
                cout << "Log:Framebuffer " << i << " Created" << endl;
            }
        }
    }
    void CreateCommandBuffer(){
        commandBuffers.resize(prerenderframes);
        VkCommandPoolCreateInfo compoolinfo{};
        compoolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        compoolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        compoolinfo.queueFamilyIndex = 1;
        vkCreateCommandPool(device, &compoolinfo, nullptr, &commandPool);
        cout << "Log:Command Pool Created" << endl;
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
        vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
        cout << "Log:Command Buffer Allocated" << endl;
    }
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent.height = resolution.y;
        renderPassInfo.renderArea.extent.width = resolution.x;
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer);
    }
    void Renewswap(){
        vkDeviceWaitIdle(device);
        CreateSwapChain(0);
        CreateImageViews(0);
        CreateRenderPass(0);
        createDescriptorSetLayout();
        CreatePipeline(0);
        CreateDepthRes();
        CreateFramebuffer(0);
        CreateVertexInput();
        createUniformBuffers();
        CreateDescriptoPool();
        createDescriptorSets();
    }
    void calculatematrix(float fov){
        proj = perspective(radians(fov), float(resolution.x / resolution.y), 0.1f, 100.0f);
        view = rotate(mat4(1.0f), rot.y, vec3(1, 0, 0));
        view = rotate(view, rot.x, vec3(0, 1, 0));
        view = translate(view, pos);
        MVP = proj * view * mat4(1.0f);
    }
    void updateUniformBuffer(float fov, uint32_t currentImage) {
        calculatematrix(fov);
        void* data;
        vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(MVP), 0, &data);
        memcpy(data, &MVP, sizeof(MVP));
        vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
    }
    void Draw(){
        glfwGetFramebufferSize(window, &resolution.x, &resolution.y);
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || oldresolution != resolution){
            oldresolution = resolution;
            Renewswap();
        }
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
        updateUniformBuffer(110, currentFrame);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        vkQueuePresentKHR(presentQueue, &presentInfo);
        currentFrame = (currentFrame + 1) % prerenderframes;
    }
    void CreateSync(){
        imageAvailableSemaphores.resize(prerenderframes);
        renderFinishedSemaphores.resize(prerenderframes);
        inFlightFences.resize(prerenderframes);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < prerenderframes; i++) {
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
        }
    }
    void Init(){
        CreateInstance();
        glfwCreateWindowSurface(instance, window, nullptr, &Surface);
        CreateDevice();
        CreateSwapChain(1);
        CreateImageViews(1);
        CreateRenderPass(1);
        createDescriptorSetLayout();
        CreatePipeline(1);
        CreateDepthRes();
        CreateFramebuffer(1);
        CreateVertexInput();
        createUniformBuffers();
        CreateDescriptoPool();
        createDescriptorSets();
        CreateCommandBuffer();
        CreateDepthRes();
        CreateSync();
        Renewswap();
    }
    void Destroy(){
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
};
