#include "RendererVulkan.hpp"
#include "../SwapchainVulkan.hpp"
#include "../RenderPassVulkan.hpp"
#include "../VulkanUtilities.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Editor/Editor.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace Brisk {
	void RendererVulkan::Create() {
		m_GpuContext = new GpuContextVulkan();
		m_GpuContext->Create();
	
		std::vector<GpuDeviceVulkan::QueueType> queueTypes;
		std::vector<GpuDeviceVulkan::DeviceFeatures> features;
        GpuDeviceVulkan::GpuRequirements req{};
		m_GpuContext->CreateDevice(req);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(GpuContextVulkan::s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(GpuContextVulkan::s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(GpuContextVulkan::s_GPUDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(GpuContextVulkan::s_GPUDevice->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
	}

    void RendererVulkan::SetupImGuiData(ImGui_ImplVulkan_InitInfo& data) {
        data.Instance = m_GpuContext->s_Instance;
        data.PhysicalDevice = m_GpuContext->s_GPUDevice->GetPhysicalDevice();
        data.Device = m_GpuContext->s_GPUDevice->GetDevice();
        data.QueueFamily = 0;
        data.Queue = m_GpuContext->s_GPUDevice->GetGraphicsQueue().Handle;
        data.DescriptorPool = m_DescriptorPool;
        data.RenderPass = m_RenderPass->GetRenderPass();
        data.ImageCount = 2;
        data.MinImageCount = 2;
        data.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    }

    //void RendererVulkan::UploadImGuiTextureAtlas() {
    //    CommandBufferVulkan singleUseCmdBuf;
    //    singleUseCmdBuf.Allocate(m_CommandPool);
    //    singleUseCmdBuf.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    //    ImGui_ImplVulkan_CreateFontsTexture();
    //    singleUseCmdBuf.End();
    //
    //    //vkDeviceWaitIdle();
    //}

    void RendererVulkan::SetupRenderingPipeline(Swapchain* swap) {
        m_Swapchain = static_cast<SwapchainVulkan*>(swap);

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_Swapchain->GetFormat().format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttatchment{};
        depthAttatchment.format = m_Swapchain->GetDepthFormat();
        depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttatchment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

        std::vector<VkAttachmentDescription> colorAttachments = { colorAttachment, depthAttatchment };

        m_RenderPass = new RenderPassVulkan();
        m_RenderPass->Create(colorAttachments, { subpass }, { dependency });
        for (int i = 0; i < m_Swapchain->GetImageCount(); i++) {
            std::vector<VkImageView> attachments = {
                m_Swapchain->GetSwapchainImageViews()[i],
                m_Swapchain->GetDepthImageView(),
            };
            m_RenderPass->CreateNAddFramebuffer(attachments, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());
        }

	    Vertices = {
	    	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	    	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	    	{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},

	    	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	    	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	    };

        m_VertexBuffer = new BufferVulkan();
	    m_VertexBuffer->Create(sizeof(Vertices[0]) * Vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	    m_VertexBuffer->Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	    m_VertexBuffer->MapMemory(Vertices);
	    m_VertexBuffer->UnMapMemory();

        m_UniformBuffer = new BufferVulkan();
        m_UniformBuffer->Create(sizeof(MVPBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_UniformBuffer->Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        m_UniformBuffer->MapMemory(&m_UniformBufferData);

        m_CommandBuffer = new CommandBufferVulkan();
        m_CommandBuffer->Allocate(m_CommandPool);

        std::vector<VkDescriptorPoolSize> poolSizes {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        };
        //poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 11;

        if (vkCreateDescriptorPool(m_GpuContext->s_GPUDevice->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        CreateDescriptorSet();
        CreateGraphicsPipeline();
    }

    void RendererVulkan::UpdateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        //
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        MVPBuffer ubo{};
        ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //float value = m_Swapchain->GetExtentWidth() / (float)m_Swapchain->GetExtentHeight();
        //ubo.Projection = glm::perspective(glm::radians(60.0f), value, 0.1f, 1000.0f);
        //ubo.Projection[1][1] *= -1;

        ubo.Model = glm::mat4(1);
        ubo.View = Engine::s_Camera->GetViewMatrix();
        ubo.Projection = Engine::s_Camera->GetProjection();
        ubo.Projection[1][1] *= -1;

        memcpy(m_UniformBufferData, &ubo, sizeof(ubo));
    }


    void RendererVulkan::CreateGraphicsPipeline() {
        m_Pipeline = new GraphicsPipelineVulkan();

        VkShaderModule vertexModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
        VkShaderModule fragmentModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleFS.spv");
        m_Pipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
        m_Pipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);

        std::vector<GraphicsPipelineVulkan::Binding> bindings;
        bindings.push_back({
            0,
            VK_VERTEX_INPUT_RATE_VERTEX,
            sizeof(Point),
            });
        std::vector<GraphicsPipelineVulkan::AttributeDescription> attributes;
        attributes.push_back({
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Point, Point::Position),
            });
        attributes.push_back({
            0,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Point, Point::Color),
            });
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        bindingDescriptions.resize(bindings.size());
        for (int i = 0; i < bindingDescriptions.size(); i++) {
            bindingDescriptions[i].binding = bindings[i].BindingIndex;
            bindingDescriptions[i].inputRate = bindings[i].InputRate;
            bindingDescriptions[i].stride = bindings[i].Stride;
        }
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.resize(attributes.size());
        for (int i = 0; i < attributeDescriptions.size(); i++) {
            attributeDescriptions[i].binding = attributes[i].BindingIndex;
            attributeDescriptions[i].location = attributes[i].Location;
            attributeDescriptions[i].format = attributes[i].Format;
            attributeDescriptions[i].offset = attributes[i].Offset;
        }
        m_Pipeline->CreateVertexInputState(bindingDescriptions, attributeDescriptions);
        m_Pipeline->CreateInputAssembly(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        m_Pipeline->CreateViewportState(1, 1);
        m_Pipeline->CreateRasterizer(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false);
        m_Pipeline->CreateMultiSampling(false, VK_SAMPLE_COUNT_1_BIT);
        m_Pipeline->CreateDepthStencil(true, true, VK_COMPARE_OP_LESS, false, false);
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        std::vector< VkPipelineColorBlendAttachmentState> colorBlendAttachments = { colorBlendAttachment };
        m_Pipeline->CrateColorBlending(colorBlendAttachments, false, VK_LOGIC_OP_COPY);
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        m_Pipeline->CreateDynamicState(dynamicStates);
        m_Pipeline->CreatePipelineLayout(m_DescriptorSetLayouts, 0);
        m_Pipeline->CreatePipeline(m_RenderPass->GetRenderPass());
    }

    void RendererVulkan::CreateDescriptorSet() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        m_DescriptorSetLayouts.resize(1);
        if (vkCreateDescriptorSetLayout(m_GpuContext->s_GPUDevice->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayouts[0]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
        allocInfo.pSetLayouts = &m_DescriptorSetLayouts[0];

        if (vkAllocateDescriptorSets(m_GpuContext->s_GPUDevice->GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_UniformBuffer->Get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MVPBuffer);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_GpuContext->s_GPUDevice->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }

	void RendererVulkan::Release() {
        m_RenderPass->Release();
        m_Pipeline->Release();
        m_VertexBuffer->Release();
        m_UniformBuffer->Release();
        
        vkDestroyCommandPool(m_GpuContext->s_GPUDevice->GetDevice(), m_CommandPool, nullptr);
        vkDestroyFence(m_GpuContext->s_GPUDevice->GetDevice(), m_InFlightFence, nullptr);
        vkDestroySemaphore(m_GpuContext->s_GPUDevice->GetDevice(), m_ImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(m_GpuContext->s_GPUDevice->GetDevice(), m_RenderFinishedSemaphore, nullptr);
        vkDestroyDescriptorPool(m_GpuContext->s_GPUDevice->GetDevice(), m_DescriptorPool, nullptr);
        for (int i = 0; i < m_DescriptorSetLayouts.size(); i++) {
            vkDestroyDescriptorSetLayout(m_GpuContext->s_GPUDevice->GetDevice(), m_DescriptorSetLayouts[i], nullptr);
        }
        Vertices.clear();

        m_GpuContext->Release();
	}

	void RendererVulkan::PreRender() {

	}

	void RendererVulkan::Render() {
        vkWaitForFences(m_GpuContext->s_GPUDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

        VkResult result = m_Swapchain->AquireNextImage(UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &m_ImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Engine::s_MainWindow->IsWindowResized()) {
            WaitDeviceIdle();
            Engine::s_Swapchain->Release();
            m_RenderPass->ReleaseFramebuffers();
            Engine::s_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_MainWindow);
            Engine::s_Swapchain->Create(Swapchain::Mode::TRIPLE_BUFFERING);
            m_Swapchain = static_cast<SwapchainVulkan*>(Engine::s_Swapchain);
            for (int i = 0; i < m_Swapchain->GetImageCount(); i++) {
                std::vector<VkImageView> attachments = {
                    m_Swapchain->GetSwapchainImageViews()[i],
                    m_Swapchain->GetDepthImageView(),
                };
                m_RenderPass->CreateNAddFramebuffer(attachments, m_Swapchain->GetExtentWidth(), m_Swapchain->GetExtentHeight());
            }
            return;
        }

        UpdateUniformBuffer(m_ImageIndex);

        //vkResetCommandBuffer(m_CommandBuffer->Get(), /*VkCommandBufferResetFlagBits*/ 0);
		m_RenderPass->BeginRenderPass(m_CommandBuffer, m_ImageIndex);
        vkCmdBindPipeline(m_CommandBuffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());
        {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentWidth());
            viewport.height = static_cast<float>(static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(m_CommandBuffer->Get(), 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();
            vkCmdSetScissor(m_CommandBuffer->Get(), 0, 1, &scissor);
        }
        {
            const VkBuffer vertexBuffers[] = { m_VertexBuffer->Get() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(m_CommandBuffer->Get(), 0, 1, vertexBuffers, offsets);

            vkCmdBindDescriptorSets(m_CommandBuffer->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);

            vkCmdDraw(m_CommandBuffer->Get(), Vertices.size(), 1, 0, 0);

            Engine::s_Editor->Update();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                static_cast<RendererVulkan*>(Engine::s_Renderer)->GetCommandBuffer(),
                VK_NULL_HANDLE);
        }
		m_RenderPass->EndRenderPass(m_CommandBuffer);

        vkResetFences(m_GpuContext->s_GPUDevice->GetDevice(), 1, &m_InFlightFence);

        {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            VkCommandBuffer cmdBufer = m_CommandBuffer->Get();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmdBufer;

            VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(m_GpuContext->s_GPUDevice->GetGraphicsQueue().Handle, 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchain() };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            presentInfo.pImageIndices = &m_ImageIndex;

            vkQueuePresentKHR(m_GpuContext->s_GPUDevice->GetGraphicsQueue().Handle, &presentInfo);
        }
	}

	void RendererVulkan::PostRender() {

	}

    void RendererVulkan::WaitDeviceIdle() {
        vkDeviceWaitIdle(m_GpuContext->s_GPUDevice->GetDevice());
    }
}