//#include "RendererVulkan.hpp"
//#include "../SwapchainVulkan.hpp"
//#include "../RenderPassVulkan.hpp"
//#include "../UtilitiesVulkan.hpp"
//#include "Graphics/Factories/SwapchainFactory.hpp"
//
//#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <chrono>
//#include <Engine/Model.hpp>
//#include <Graphics/Vulkan/Defines.h>
//
//namespace Brisk {
//    Model* m_Model;
//
//    static void CheckAvailableExtensions() {
//        VkResult result;
//
//        /*
//         * From the link above:
//         * If `pProperties` is NULL, then the number of extensions properties
//         * available is returned in `pPropertyCount`.
//         *
//         * Basically, gets the number of extensions.
//         */
//        uint32_t count = 0;
//        result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
//        if (result != VK_SUCCESS) {
//            // Throw an exception or log the error
//        }
//
//        std::vector<VkExtensionProperties> extensionProperties(count);
//
//        // Get the extensions
//        result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
//        if (result != VK_SUCCESS) {
//            assert(false);
//        }
//
//        //std::set<std::string> extensions;
//        std::cout << "Availble Instance Extension:" << std::endl;
//        for (auto& extension : extensionProperties) {
//            std::cout << extension.extensionName << std::endl;
//        }
//
//        std::vector<VkLayerProperties> layers;
//
//        count = 0;
//        result = vkEnumerateInstanceLayerProperties(&count, nullptr);
//        if (result != VK_SUCCESS) {
//            assert(false);
//        }
//
//        layers.resize(count);
//        result = vkEnumerateInstanceLayerProperties(&count, layers.data());
//        if (result != VK_SUCCESS) {
//            assert(false);
//        }
//        std::cout << "Availble Instance Layers:" << std::endl;
//        for (auto& layer : layers) {
//            std::cout << layer.layerName << std::endl;
//        }
//
//    }
//    void TransitionSwapchainImageLayout(
//        VkCommandBuffer commandBuffer,
//        VkImage image,
//        VkImageLayout oldLayout,
//        VkImageLayout newLayout,
//        VkPipelineStageFlags srcStage,
//        VkPipelineStageFlags dstStage
//    ) {
//        VkImageMemoryBarrier barrier = {};
//        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//        barrier.image = image;
//        barrier.oldLayout = oldLayout;
//        barrier.newLayout = newLayout;
//        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        barrier.subresourceRange.baseMipLevel = 0;
//        barrier.subresourceRange.levelCount = 1;
//        barrier.subresourceRange.baseArrayLayer = 0;
//        barrier.subresourceRange.layerCount = 1;
//
//        // Determine access masks
//        switch (oldLayout) {
//        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
//            barrier.srcAccessMask = 0;
//            break;
//        default:
//            barrier.srcAccessMask = 0;
//            break;
//        }
//
//        switch (newLayout) {
//        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
//            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
//            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
//            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//            break;
//        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
//            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//            break;
//        default:
//            barrier.dstAccessMask = 0;
//            break;
//        }
//
//        vkCmdPipelineBarrier(
//            commandBuffer,
//            srcStage,
//            dstStage,
//            0,
//            0, nullptr,
//            0, nullptr,
//            1, &barrier
//        );
//    }
//
//    //void GpuContextVulkan::Release() {
//    //    s_GPUDevice->Release();
//    //    vkDestroySurfaceKHR(s_Instance, s_Surface->GetRef(), nullptr);
//    //    vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugMessenger, nullptr);
//    //    vkDestroyInstance(s_Instance, nullptr);
//    //}
//
//    void RendererVulkan::Init() {
//        VkSemaphoreCreateInfo semaphoreInfo{};
//        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//        VkFenceCreateInfo fenceInfo{};
//        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//        if (vkCreateSemaphore(s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
//            vkCreateSemaphore(s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderToTextureFinished) != VK_SUCCESS ||
//            vkCreateSemaphore(s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderToSwapchainFinished) != VK_SUCCESS ||
//            vkCreateFence(s_GPUDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create synchronization objects for a frame!");
//        }
//
//        VkCommandPoolCreateInfo poolInfo{};
//        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//        poolInfo.queueFamilyIndex = s_GPUDevice->GetGraphicsQueue().FamilyIndex;
//
//        if (vkCreateCommandPool(s_GPUDevice->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create command pool!");
//        }
//    }
//
//    //void RendererVulkan::SetupImGuiData(ImGui_ImplVulkan_InitInfo& data) {
//    //    data.Instance = s_Instance;
//    //    data.PhysicalDevice = s_GPUDevice->GetPhysicalDevice();
//    //    data.Device = s_GPUDevice->GetDevice();
//    //    data.QueueFamily = 0;
//    //    data.Queue = s_GPUDevice->GetGraphicsQueue().Handle;
//    //    data.DescriptorPool = m_DescriptorPool;
//    //    data.RenderPass = m_RenderPass->GetRenderPass();
//    //    data.ImageCount = 2;
//    //    data.MinImageCount = 2;
//    //    data.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
//    //}
//
//    void RendererVulkan::CreateOffscreenResources() {
//        //m_Viewport.pSceneTexture = static_cast<TextureVulkan*>(m_RenderTarget.pTexture);
//
//        // Creating renderpass
//        {
//            VkAttachmentDescription colorAttachment{};
//            colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
//            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//            VkAttachmentDescription depthAttatchment{};
//            depthAttatchment.format = m_Swapchain->GetDepthFormat();
//            depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
//            depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//            depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//            depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//            depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//            depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            depthAttatchment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//            VkAttachmentReference colorAttachmentRef{};
//            colorAttachmentRef.attachment = 0;
//            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//            VkAttachmentReference depthAttachmentRef{};
//            depthAttachmentRef.attachment = 1;
//            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//            VkSubpassDescription subpass{};
//            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//            subpass.colorAttachmentCount = 1;
//            subpass.pColorAttachments = &colorAttachmentRef;
//            subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//            // Subpass dependencies for layout transitions
//            std::array<VkSubpassDependency, 2> dependencies;
//
//            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//            dependencies[0].dstSubpass = 0;
//            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//            dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//            dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
//            dependencies[0].dependencyFlags = 0;
//
//            dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
//            dependencies[1].dstSubpass = 0;
//            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//            dependencies[1].srcAccessMask = 0;
//            dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
//            dependencies[1].dependencyFlags = 0;
//
//            std::vector<VkAttachmentDescription> colorAttachments = { colorAttachment, depthAttatchment };
//
//            m_Viewport.pRenderpass = new RenderPassVulkan();
//            m_Viewport.pRenderpass->Create(colorAttachments, { subpass }, { dependencies[0], dependencies[1] });
//            std::vector<VkImageView> attachments = {
//                m_Viewport.pSceneTexture->GetView(),
//                m_Swapchain->GetDepthImageView(),
//            };
//            m_Viewport.pRenderpass->CreateNAddFramebuffer(attachments, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());
//        }
//
//        // Pipeline
//        {
//            m_Viewport.pPipeline = new GraphicsPipelineVulkan();
//
//            VkShaderModule vertexModule = UtilitiesVulkan::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
//            VkShaderModule fragmentModule = UtilitiesVulkan::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleFS.spv");
//            m_Viewport.pPipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
//            m_Viewport.pPipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);
//
//            std::vector<GraphicsPipelineVulkan::Binding> bindings;
//            bindings.push_back({
//                0,
//                VK_VERTEX_INPUT_RATE_VERTEX,
//                sizeof(Vertex),
//                });
//            std::vector<GraphicsPipelineVulkan::AttributeDescription> attributes;
//            attributes.push_back({
//                0,
//                0,
//                VK_FORMAT_R32G32B32_SFLOAT,
//                offsetof(Vertex, Vertex::pos),
//                });
//            attributes.push_back({
//                0,
//                1,
//                VK_FORMAT_R32G32B32_SFLOAT,
//                offsetof(Vertex, Vertex::normal),
//                });
//            attributes.push_back({
//                0,
//                2,
//                VK_FORMAT_R32G32_SFLOAT,
//                offsetof(Vertex, Vertex::uv0),
//                });
//            attributes.push_back({
//                0,
//                3,
//                VK_FORMAT_R32G32_SFLOAT,
//                offsetof(Vertex, Vertex::uv1),
//                });
//            attributes.push_back({
//                0,
//                4,
//                VK_FORMAT_R32G32B32_SFLOAT,
//                offsetof(Vertex, Vertex::color),
//                });
//            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
//            bindingDescriptions.resize(bindings.size());
//            for (int i = 0; i < bindingDescriptions.size(); i++) {
//                bindingDescriptions[i].binding = bindings[i].BindingIndex;
//                bindingDescriptions[i].inputRate = bindings[i].InputRate;
//                bindingDescriptions[i].stride = bindings[i].Stride;
//            }
//            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
//            attributeDescriptions.resize(attributes.size());
//            for (int i = 0; i < attributeDescriptions.size(); i++) {
//                attributeDescriptions[i].binding = attributes[i].BindingIndex;
//                attributeDescriptions[i].location = attributes[i].Location;
//                attributeDescriptions[i].format = attributes[i].Format;
//                attributeDescriptions[i].offset = attributes[i].Offset;
//            }
//            m_Viewport.pPipeline->CreateVertexInputState(bindingDescriptions, attributeDescriptions);
//            m_Viewport.pPipeline->CreateInputAssembly(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//            m_Viewport.pPipeline->CreateViewportState(1, 1);
//            m_Viewport.pPipeline->CreateRasterizer(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false);
//            m_Viewport.pPipeline->CreateMultiSampling(false, VK_SAMPLE_COUNT_1_BIT);
//            m_Viewport.pPipeline->CreateDepthStencil(true, true, VK_COMPARE_OP_LESS, false, false);
//            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//            colorBlendAttachment.blendEnable = VK_FALSE;
//            std::vector< VkPipelineColorBlendAttachmentState> colorBlendAttachments = { colorBlendAttachment };
//            m_Viewport.pPipeline->CrateColorBlending(colorBlendAttachments, false, VK_LOGIC_OP_COPY);
//            std::vector<VkDynamicState> dynamicStates = {
//                VK_DYNAMIC_STATE_VIEWPORT,
//                VK_DYNAMIC_STATE_SCISSOR
//            };
//            m_Viewport.pPipeline->CreateDynamicState(dynamicStates);
//            m_Viewport.pPipeline->CreatePipelineLayout(m_DescriptorSetLayouts, 0);
//            m_Viewport.pPipeline->CreatePipeline(m_Viewport.pRenderpass->GetRenderPass());
//        }
//    }
//
//    void RendererVulkan::SetupRenderingPipeline(Swapchain* swap) {
//        m_Swapchain = static_cast<SwapchainVulkan*>(swap);
//
//        VkAttachmentDescription colorAttachment{};
//        colorAttachment.format = m_Swapchain->GetFormat().format;
//        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//        VkAttachmentDescription depthAttatchment{};
//        depthAttatchment.format = m_Swapchain->GetDepthFormat();
//        depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
//        depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//        depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//        depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//        depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//        depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        depthAttatchment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//        VkAttachmentReference colorAttachmentRef{};
//        colorAttachmentRef.attachment = 0;
//        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//        VkAttachmentReference depthAttachmentRef{};
//        depthAttachmentRef.attachment = 1;
//        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//        VkSubpassDescription subpass{};
//        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//        subpass.colorAttachmentCount = 1;
//        subpass.pColorAttachments = &colorAttachmentRef;
//        subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//        VkSubpassDependency dependency{};
//        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//        dependency.dstSubpass = 0;
//        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//        dependency.srcAccessMask = 0;
//        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//        std::vector<VkAttachmentDescription> colorAttachments = { colorAttachment, depthAttatchment };
//
//        m_RenderPass = new RenderPassVulkan();
//        m_RenderPass->Create(colorAttachments, { subpass }, { dependency });
//        for (int i = 0; i < m_Swapchain->GetImageCount(); i++) {
//            std::vector<VkImageView> attachments = {
//                m_Swapchain->GetSwapchainImageViews()[i],
//                m_Swapchain->GetDepthImageView(),
//            };
//            m_RenderPass->CreateNAddFramebuffer(attachments, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());
//        }
//
//        m_VertexBuffer = new BufferVulkan();
//        //m_VertexBuffer->Create(sizeof(Vertices[0]) * Vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//        m_VertexBuffer->Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//        //m_VertexBuffer->MapMemory(Vertices);
//        m_VertexBuffer->UnMapMemory();
//
//        m_UniformBuffer = new BufferVulkan();
//        m_UniformBuffer->Create(sizeof(MVPBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
//        m_UniformBuffer->Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//        m_UniformBuffer->MapMemory(&m_UniformBufferData);
//
//        m_CommandBuffer = new CommandBufferVulkan();
//        m_CommandBuffer->Allocate(m_CommandPool);
//
//        m_ImGuiCommandBuffer = new CommandBufferVulkan();
//        m_ImGuiCommandBuffer->Allocate(m_CommandPool);
//
//        std::vector<VkDescriptorPoolSize> poolSizes {
//            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
//            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
//        };
//
//        VkDescriptorPoolCreateInfo poolInfo{};
//        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//        poolInfo.pPoolSizes = poolSizes.data();
//        poolInfo.maxSets = 11;
//
//        if (vkCreateDescriptorPool(s_GPUDevice->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create descriptor pool!");
//        }
//
//        CreateDescriptorSet();
//        CreateGraphicsPipeline();
//    }
//
//    void RendererVulkan::UpdateUniformBuffer(uint32_t currentImage) {
//        static auto startTime = std::chrono::high_resolution_clock::now();
//        //
//        auto currentTime = std::chrono::high_resolution_clock::now();
//        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
//
//        MVPBuffer ubo{};
//        ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//
//        ubo.Model = glm::mat4(1);
//        if (Engine::m_ActiveScene->SelectedElement != -1)
//            ubo.Model = Engine::m_ActiveScene->Elements[Engine::m_ActiveScene->SelectedElement].GetModule<Transform>()->GetMat();
//
//        ubo.View = Engine::s_Camera->GetViewMatrix();
//        ubo.Projection = Engine::s_Camera->GetProjection();
//        ubo.Projection[1][1] *= -1;
//
//        memcpy(m_UniformBufferData, &ubo, sizeof(ubo));
//    }
//
//    void RendererVulkan::CreateGraphicsPipeline() {
//        m_Pipeline = new GraphicsPipelineVulkan();
//
//        VkShaderModule vertexModule = UtilitiesVulkan::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
//        VkShaderModule fragmentModule = UtilitiesVulkan::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleFS.spv");
//        m_Pipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
//        m_Pipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);
//
//        std::vector<GraphicsPipelineVulkan::Binding> bindings;
//        bindings.push_back({
//            0,
//            VK_VERTEX_INPUT_RATE_VERTEX,
//            sizeof(Vertex),
//            });
//        std::vector<GraphicsPipelineVulkan::AttributeDescription> attributes;
//        attributes.push_back({
//            0,
//            0,
//            VK_FORMAT_R32G32B32_SFLOAT,
//            offsetof(Vertex, Vertex::pos),
//            });
//        attributes.push_back({
//            0,
//            1,
//            VK_FORMAT_R32G32B32_SFLOAT,
//            offsetof(Vertex, Vertex::normal),
//            });
//        attributes.push_back({
//            0,
//            2,
//            VK_FORMAT_R32G32_SFLOAT,
//            offsetof(Vertex, Vertex::uv0),
//            });
//        attributes.push_back({
//            0,
//            3,
//            VK_FORMAT_R32G32_SFLOAT,
//            offsetof(Vertex, Vertex::uv1),
//            });
//        attributes.push_back({
//            0,
//            4,
//            VK_FORMAT_R32G32B32_SFLOAT,
//            offsetof(Vertex, Vertex::color),
//            });
//        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
//        bindingDescriptions.resize(bindings.size());
//        for (int i = 0; i < bindingDescriptions.size(); i++) {
//            bindingDescriptions[i].binding = bindings[i].BindingIndex;
//            bindingDescriptions[i].inputRate = bindings[i].InputRate;
//            bindingDescriptions[i].stride = bindings[i].Stride;
//        }
//        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
//        attributeDescriptions.resize(attributes.size());
//        for (int i = 0; i < attributeDescriptions.size(); i++) {
//            attributeDescriptions[i].binding = attributes[i].BindingIndex;
//            attributeDescriptions[i].location = attributes[i].Location;
//            attributeDescriptions[i].format = attributes[i].Format;
//            attributeDescriptions[i].offset = attributes[i].Offset;
//        }
//        m_Pipeline->CreateVertexInputState(bindingDescriptions, attributeDescriptions);
//        m_Pipeline->CreateInputAssembly(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//        m_Pipeline->CreateViewportState(1, 1);
//        m_Pipeline->CreateRasterizer(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false);
//        m_Pipeline->CreateMultiSampling(false, VK_SAMPLE_COUNT_1_BIT);
//        m_Pipeline->CreateDepthStencil(true, true, VK_COMPARE_OP_LESS, false, false);
//        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//        colorBlendAttachment.blendEnable = VK_FALSE;
//        std::vector< VkPipelineColorBlendAttachmentState> colorBlendAttachments = { colorBlendAttachment };
//        m_Pipeline->CrateColorBlending(colorBlendAttachments, false, VK_LOGIC_OP_COPY);
//        std::vector<VkDynamicState> dynamicStates = {
//            VK_DYNAMIC_STATE_VIEWPORT,
//            VK_DYNAMIC_STATE_SCISSOR
//        };
//        m_Pipeline->CreateDynamicState(dynamicStates);
//        m_Pipeline->CreatePipelineLayout(m_DescriptorSetLayouts, 0);
//        m_Pipeline->CreatePipeline(m_RenderPass->GetRenderPass());
//
//        m_Model = new Model();
//        m_Model->Load("../Data/Models/Cube/Cube.gltf");
//        //m_Model->Load("../Data/Models/gun/gun.obj");
//        //m_Model->Load("../Data/Models/Fox/glTF/Fox.gltf");
//    }
//
//    void RendererVulkan::CreateDescriptorSet() {
//        VkDescriptorSetLayoutBinding uboLayoutBinding{};
//        uboLayoutBinding.binding = 0;
//        uboLayoutBinding.descriptorCount = 1;
//        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        uboLayoutBinding.pImmutableSamplers = nullptr;
//        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//
//        VkDescriptorSetLayoutCreateInfo layoutInfo{};
//        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//        layoutInfo.bindingCount = 1;
//        layoutInfo.pBindings = &uboLayoutBinding;
//
//        m_DescriptorSetLayouts.resize(1);
//        if (vkCreateDescriptorSetLayout(s_GPUDevice->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayouts[0]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create descriptor set layout!");
//        }
//
//        VkDescriptorSetAllocateInfo allocInfo{};
//        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//        allocInfo.descriptorPool = m_DescriptorPool;
//        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
//        allocInfo.pSetLayouts = &m_DescriptorSetLayouts[0];
//
//        if (vkAllocateDescriptorSets(s_GPUDevice->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
//            throw std::runtime_error("failed to allocate descriptor sets!");
//        }
//
//        VkDescriptorBufferInfo bufferInfo{};
//        bufferInfo.buffer = m_UniformBuffer->Get();
//        bufferInfo.offset = 0;
//        bufferInfo.range = sizeof(MVPBuffer);
//
//        VkWriteDescriptorSet descriptorWrite{};
//        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//        descriptorWrite.dstSet = m_DescriptorSet;
//        descriptorWrite.dstBinding = 0;
//        descriptorWrite.dstArrayElement = 0;
//        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        descriptorWrite.descriptorCount = 1;
//        descriptorWrite.pBufferInfo = &bufferInfo;
//
//        vkUpdateDescriptorSets(s_GPUDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
//    }
//
//    void RendererVulkan::Release() {
//
//    }
//
//    void RendererVulkan::PreRender() {
//
//    }
//
//    void RendererVulkan::Render() {
//        vkWaitForFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
//
//        VkResult result = m_Swapchain->AquireNextImage(UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &m_ImageIndex);
//
//        //if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Engine::s_MainWindow->IsWindowResized()) {
//        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
//            WaitDeviceIdle();
//            Engine::s_Swapchain->Release();
//            m_RenderPass->ReleaseFramebuffers();
//            delete Engine::s_Swapchain;
//            Engine::s_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
//            Engine::s_Swapchain->Create(Swapchain::Mode::TRIPLE_BUFFERING);
//            m_Swapchain = static_cast<SwapchainVulkan*>(Engine::s_Swapchain);
//            for (int i = 0; i < m_Swapchain->GetImageCount(); i++) {
//                std::vector<VkImageView> attachments = {
//                    m_Swapchain->GetSwapchainImageViews()[i],
//                    m_Swapchain->GetDepthImageView(),
//                };
//                m_RenderPass->CreateNAddFramebuffer(attachments, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());
//            }
//            return;
//        }
//        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//            throw std::runtime_error("Failed to acquire swapchain image");
//        }
//
//        vkResetFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence);
//        UpdateUniformBuffer(m_ImageIndex);
//
//        vkResetCommandBuffer(m_CommandBuffer->Get(), /*VkCommandBufferResetFlagBits*/ 0);
//
//        m_Viewport.pRenderpass->BeginRenderPass(m_CommandBuffer, m_ImageIndex);
//        vkCmdBindPipeline(m_CommandBuffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Viewport.pPipeline->GetPipeline());
//        {
//            VkViewport viewport{};
//            viewport.x = 0.0f;
//            viewport.y = 0.0f;
//            viewport.width = static_cast<float>(m_Swapchain->GetExtentWidth());
//            viewport.height = static_cast<float>(m_Swapchain->GetExtentHeight());
//            viewport.minDepth = 0.0f;
//            viewport.maxDepth = 1.0f;
//            vkCmdSetViewport(m_CommandBuffer->Get(), 0, 1, &viewport);
//
//            VkRect2D scissor{};
//            scissor.offset = { 0, 0 };
//            scissor.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();
//            vkCmdSetScissor(m_CommandBuffer->Get(), 0, 1, &scissor);
//        }
//        {
//            const VkBuffer vertexBuffers[] = { m_Model->m_VertexBuffer->Get() };
//            VkDeviceSize offsets[] = { 0 };
//            vkCmdBindVertexBuffers(m_CommandBuffer->Get(), 0, 1, vertexBuffers, offsets);
//            vkCmdBindDescriptorSets(m_CommandBuffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Viewport.pPipeline->GetLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
//            vkCmdDraw(m_CommandBuffer->Get(), m_Model->m_VertexBuffer->GetSize(), 1, 0, 0);
//        }
//        m_Viewport.pRenderpass->EndRenderPass(m_CommandBuffer, false);
//
//        {
//            VkImageMemoryBarrier imageMemoryBarrier = {};
//            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            imageMemoryBarrier.image = m_Viewport.pSceneTexture->GetImage();  // The VkImage you're transitioning
//
//            // Define which aspects of the image are affected (typically color)
//            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
//            imageMemoryBarrier.subresourceRange.levelCount = 1;
//            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
//            imageMemoryBarrier.subresourceRange.layerCount = 1;
//
//            // Source access mask � operations that must complete before the transition
//            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//            // Destination access mask � operations that will happen after the transition
//            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//
//            VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//            VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//
//            vkCmdPipelineBarrier(
//                m_CommandBuffer->Get(),
//                srcStage,
//                dstStage,
//                0,
//                0, nullptr,
//                0, nullptr,
//                1, &imageMemoryBarrier
//            );
//
//            m_CommandBuffer->End();
//
//            vkResetFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence);
//
//            VkSubmitInfo submitInfo{};
//            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//            std::vector<VkCommandBuffer> cmdBufers = { m_CommandBuffer->Get() };
//            submitInfo.commandBufferCount = 1;
//            submitInfo.pCommandBuffers = cmdBufers.data();
//
//            VkSemaphore signalSemaphores[] = { m_RenderToTextureFinished };
//            submitInfo.signalSemaphoreCount = 1;
//            submitInfo.pSignalSemaphores = signalSemaphores;
//
//            if (vkQueueSubmit(s_GPUDevice->GetGraphicsQueue().Handle, 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) {
//                throw std::runtime_error("failed to submit draw command buffer!");
//            }
//
//            vkDeviceWaitIdle(s_GPUDevice->GetDevice());
//
//            vkWaitForFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
//            vkResetFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence);
//
//            m_RenderPass->BeginRenderPass(m_ImGuiCommandBuffer, m_ImageIndex);
//            Engine::s_Editor->Update();
//            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImGuiCommandBuffer->Get(), VK_NULL_HANDLE);
//            m_RenderPass->EndRenderPass(m_ImGuiCommandBuffer);
//        }
//
//        {
//            VkFenceCreateInfo CI{};
//            CI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//            vkCreateFence(s_GPUDevice->GetDevice(), &CI, nullptr, &m_InFlightFence);
//            VkSubmitInfo submitInfo{};
//            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//            VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore, m_RenderToTextureFinished };
//            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//            submitInfo.waitSemaphoreCount = 2;
//            submitInfo.pWaitSemaphores = waitSemaphores;
//            submitInfo.pWaitDstStageMask = waitStages;
//
//            std::vector<VkCommandBuffer> cmdBufers = { m_ImGuiCommandBuffer->Get() };
//            submitInfo.commandBufferCount = 1;
//            submitInfo.pCommandBuffers = cmdBufers.data();
//
//            VkSemaphore signalSemaphores[] = { m_RenderToSwapchainFinished };
//            submitInfo.signalSemaphoreCount = 1;
//            submitInfo.pSignalSemaphores = signalSemaphores;
//
//            if (vkQueueSubmit(s_GPUDevice->GetGraphicsQueue().Handle, 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) {
//                throw std::runtime_error("failed to submit draw command buffer!");
//            }
//            vkWaitForFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
//            vkResetFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence);
//
//            VkPresentInfoKHR presentInfo{};
//            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//            presentInfo.waitSemaphoreCount = 1;
//            presentInfo.pWaitSemaphores = signalSemaphores;
//
//            VkSwapchainKHR swapChains[] = { m_Swapchain->GetSwapchain() };
//            presentInfo.swapchainCount = 1;
//            presentInfo.pSwapchains = swapChains;
//
//            presentInfo.pImageIndices = &m_ImageIndex;
//
//            vkQueuePresentKHR(s_GPUDevice->GetGraphicsQueue().Handle, &presentInfo);
//        }
//    }
//
//    void RendererVulkan::PostRender() {
//
//    }
//
//    void RendererVulkan::WaitDeviceIdle() {
//        vkDeviceWaitIdle(s_GPUDevice->GetDevice());
//    }
//}