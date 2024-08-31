#include "RendererVulkan.hpp"
#include "../SwapchainVulkan.hpp"
#include "../RenderPassVulkan.hpp"
#include "../VulkanUtilities.hpp"

namespace Brisk {
	void RendererVulkan::Create() {
		m_GpuContext = new GpuContextVulkan();
		m_GpuContext->Create();
	
		std::vector<GpuDeviceVulkan::QueueType> queueTypes;
		std::vector<GpuDeviceVulkan::DeviceFeatures> features;
		GpuDeviceVulkan::GpuRequirements req;
		m_GpuContext->CreateDevice(req);
	}

    void RendererVulkan::SetupRenderingPipeline(const Swapchain* swap) {
        const SwapchainVulkan* swapchain = static_cast<const SwapchainVulkan*>(swap);

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchain->GetFormat().format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttatchment{};
        depthAttatchment.format = swapchain->GetDepthFormat();
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

        std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttatchment };

        RenderPassVulkan* m_RenderPass = new RenderPassVulkan();
        m_RenderPass->Create(attachments, { subpass }, { dependency });
        for (int i = 0; i < swapchain->GetImageCount(); i++) {
            std::vector<VkImageView> attachments = {
                swapchain->GetSwapchainImageViews()[i],
                swapchain->GetDepthImageView(),
            };
            m_RenderPass->CreateNAddFramebuffer(attachments, swapchain->GetExtentWidth(), swapchain->GetExtentHeight());
        }

        GraphicsPipelineVulkan* pipeline = new GraphicsPipelineVulkan();
        
        VkShaderModule vertexModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "");
        VkShaderModule fragmentModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "");
        pipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
        pipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);

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
        pipeline->CreateVertexInputState(bindings, attributes);


        pipeline->CreateInputAssembly(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        pipeline->CreateViewportState(1, 1);

        pipeline->CreateRasterizer(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, false);


        pipeline->CreateMultiSampling(false, VK_SAMPLE_COUNT_1_BIT);


        pipeline->CreateDepthStencil(true, true, VK_COMPARE_OP_LESS, false, false);

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        pipeline->CrateColorBlending({ colorBlendAttachment }, false, VK_LOGIC_OP_COPY);

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        pipeline->CreateDynamicState(dynamicStates);

        pipeline->CreatePipelineLayout(0, 0);

        pipeline->CreatePipeline(m_RenderPass->GetRenderPass());

        //

	    std::vector<Point> vertices = {
	    	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	    	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	    	{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},

	    	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	    	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	    };

        BufferVulkan* m_VertexBuffer = new BufferVulkan();
	    m_VertexBuffer->Create(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	    m_VertexBuffer->Allocate();
	    m_VertexBuffer->MapMemory(vertices);
    }

	void RendererVulkan::Release() {

	}

	void RendererVulkan::PreRender() {

	}

	void RendererVulkan::Render() {
    	if (Engine::s_GPUContext->Sync())
			return;

		m_RenderPass->BeginRenderPass();
		m_RenderPass->BindPipeline(m_DefaultGraphicsPipeline);

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->PrepreFrame(
			static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer());
		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Draw(
			static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer(), *m_VertexBuffer);

		m_RenderPass->EndRenderPass();

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Submit(
			static_cast<RenderPassVulkan*>(m_RenderPass));

	}

	void RendererVulkan::PostRender() {

	}
}