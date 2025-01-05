#include "Renderer.hpp"
#include "RHI.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Engine/Model.hpp"
#include "RendererAPI.hpp"
#include "Graphics/Vulkan/CommandBufferVulkan.hpp"
#include "RenderCommand.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Component.hpp"
#include "ShaderModule.hpp"
#include "Fence.hpp"
#include "semaphore.hpp"
#include "Queue.hpp"

namespace Brisk
{
    std::shared_ptr<Semaphore> ImageAvailableSemaphore;
    std::shared_ptr<Semaphore> RenderFinishedSemaphore;
    std::shared_ptr<Fence> fence;
    uint32_t imageIndex;
    std::shared_ptr<CommandBuffer> cmd;
    VkCommandPool m_CommandPool;
    RenderCommand command;

    std::shared_ptr<Pipeline> pipeline;

    void Renderer::Init()
    {
        m_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_Application->GetWindow());
        m_Swapchain->Create(Swapchain::DOUBLE_BUFFERING);

        std::shared_ptr<DescriptorLayout> materialLayout = DescriptorLayout::Create();
        materialLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::ResourceBuffer);
        materialLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::ResourceBuffer);
        materialLayout->AddBindingLayout(2, 1, GPUResource::ResourceType::ResourceTexture);
        materialLayout->AddBindingLayout(3, 1, GPUResource::ResourceType::ResourceTexture);
        materialLayout->AddBindingLayout(4, 1, GPUResource::ResourceType::ResourceTexture);
        materialLayout->AddBindingLayout(5, 1, GPUResource::ResourceType::ResourceTexture);
        materialLayout->AddBindingLayout(6, 1, GPUResource::ResourceType::ResourceTexture);
        materialLayout->Init();

        std::shared_ptr<DescriptorLayout> pbrLayout = DescriptorLayout::Create();
        pbrLayout->AddBindingLayout(0, 1, GPUResource::ResourceType::ResourceTexture);
        pbrLayout->AddBindingLayout(1, 1, GPUResource::ResourceType::ResourceTexture);
        pbrLayout->AddBindingLayout(2, 1, GPUResource::ResourceType::ResourceTexture);
        pbrLayout->Init();

        std::shared_ptr<ShaderModule> vertexShaderModule = ShaderModule::Create();
        vertexShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleVS.spv", Pipeline::ShaderStage::VERTEX));

        std::shared_ptr<ShaderModule> fragmentShaderModule = ShaderModule::Create();
        fragmentShaderModule->Init(std::make_pair("Shaders/Vulkan/Compiled/TriangleFS.spv", Pipeline::ShaderStage::FRAGMENT));

        Pipeline::PipelineSpecs pipelineSpecs{};
        RenderPass::RenderPassSpecs renderPassSpecs;
        renderPassSpecs.pAttachments =
            {
                {0, Core::Format::FORMAT_B8G8R8A8_UNORM, true, RenderPass::AttachmentType::Swapchain},
                {1, Core::Format::FORMAT_D32_SFLOAT, true, RenderPass::AttachmentType::Depth}};

        Pipeline::VertexDataLayout vertexLayout;
        vertexLayout.pBinding = 0;
        vertexLayout.pStride = sizeof(MeshData);
        vertexLayout.pAttributes = {
            {0, 0, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Position)},
            {0, 1, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Normal)},
            {0, 2, Core::Format::FORMAT_R32G32_SFLOAT, offsetof(MeshData, MeshData::UV0)},
            {0, 3, Core::Format::FORMAT_R32G32_SFLOAT, offsetof(MeshData, MeshData::UV1)},
            {0, 4, Core::Format::FORMAT_R32G32B32_SFLOAT, offsetof(MeshData, MeshData::Color)},
        };
        pipelineSpecs.pLayout = vertexLayout;
        pipelineSpecs.pRenderPass = RenderPass::Create();
        pipelineSpecs.pRenderPass->Init(renderPassSpecs);

        pipelineSpecs.pDescriptorLayouts.push_back(materialLayout);
        pipelineSpecs.pDescriptorLayouts.push_back(pbrLayout);

        pipelineSpecs.pShaderModules.push_back(vertexShaderModule);
        pipelineSpecs.pShaderModules.push_back(fragmentShaderModule);

        pipelineSpecs.pDepthClampEnable = false;
        pipelineSpecs.pRasterizationDiscardEnable = false;
        pipelineSpecs.pPolygoneMode = Pipeline::POLYGON_MODE_FILL;
        pipelineSpecs.pLineWidth = 1.0f;
        pipelineSpecs.pCullMode = Pipeline::CullMode::BACK;
        pipelineSpecs.pFrontFace = Pipeline::FrontFace::CLOCKWISE;
        pipelineSpecs.pDepthBiasEnable = false;
        pipelineSpecs.pDepthTestEnable = true;
        pipelineSpecs.pDepthWriteEnable = true;
        pipelineSpecs.pCompareOp = Pipeline::COMPARE_OP_LESS;
        pipelineSpecs.pDepthBoundsTestEnable = false;
        pipelineSpecs.pStencilTestEnable = false;

        pipeline = Pipeline::Create();
        pipeline->Init(pipelineSpecs);

        cmd = CommandBuffer::Create();

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        fence = Fence::Create();
        fence->Init();

        ImageAvailableSemaphore = Semaphore::Create();
        ImageAvailableSemaphore->Init();

        RenderFinishedSemaphore = Semaphore::Create();
        RenderFinishedSemaphore->Init();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().FamilyIndex;

        if (vkCreateCommandPool(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }

        std::static_pointer_cast<CommandBufferVulkan>(cmd)->Allocate(m_CommandPool);
    }

    void Renderer::RenderScene(float deltaTime)
    {
        // vkWaitForFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
        fence->Wait();

        m_Swapchain->AquireNextImage(UINT64_MAX, ImageAvailableSemaphore, fence, &imageIndex);
        // vkResetCommandBuffer(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), /*VkCommandBufferResetFlagBits*/ 0);
        cmd->Reset();
        cmd->Bind();
        pipeline->m_Specs.pRenderPass->Begin(cmd, imageIndex);
        pipeline->Bind(cmd);

        cmd->RecordCommand([=]()
                           {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_Swapchain->GetExtentWidth());
            viewport.height = static_cast<float>(m_Swapchain->GetExtentHeight());
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &viewport); });

        cmd->RecordCommand([&]()
                           {
            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->GetExtent();
            vkCmdSetScissor(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, &scissor); });

        fence->Reset();
        // vkResetFences(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), 1, &fence);

        // const VkBuffer vertexBuffers[] = { std::static_pointer_cast<BufferVulkan>(m_VertexBuffer)->Get() };
        VkDeviceSize offsets[] = {0};
        // vkCmdBindVertexBuffers(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), 0, 1, vertexBuffers, offsets);
        // vkCmdBindDescriptorSets(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::static_pointer_cast<PipelineVulkan>(pipeline)->GetLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
        // RenderCommand::Draw(cmd, static_cast<uint32_t>(vertices.size()), 0);

        pipeline->m_Specs.pRenderPass->End(cmd);
        cmd->UnBind();

        std::shared_ptr<Queue> queue;

        Queue::SubmitInfo submitInfo{};
        submitInfo.pSignalSemaphores.push_back(ImageAvailableSemaphore);
        submitInfo.pWaitStages.push_back(Queue::WaitStage::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        submitInfo.pCmdBuffers.push_back(cmd);

        submitInfo.pSignalSemaphores.push_back(RenderFinishedSemaphore);

        queue->Submit(submitInfo, fence);
        //if (vkQueueSubmit(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, 1, &submitInfo, fence) != VK_SUCCESS)
        //{
        //    throw std::runtime_error("failed to submit draw command buffer!");
        //}

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {std::static_pointer_cast<SwapchainVulkan>(m_Swapchain)->GetSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue().Handle, &presentInfo);
    }

    std::unique_ptr<Renderer> Renderer::Create()
    {
        return std::make_unique<Renderer>();
    }
}