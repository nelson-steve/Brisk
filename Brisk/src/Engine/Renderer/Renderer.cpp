#include "Renderer.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk {
	std::unique_ptr<Renderer> Renderer::Create() {
		return std::make_unique<RendererVulkan>();
	}

	void Renderer::Init() {
		Attachment color = {
			format,
			SAMPLE_COUNT_1_BIT,
			LOAD_OP_CLEAR,
			STORE_OP_STORE,
			 LOAD_OP_DONT_CARE,
			STORE_OP_DONT_CARE,
			IMAGE_LAYOUT_UNDEFINED,
			IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		Attachment depth = {
			format,
			SAMPLE_COUNT_1_BIT,
			LOAD_OP_CLEAR,
			STORE_OP_STORE,
			 LOAD_OP_DONT_CARE,
			STORE_OP_DONT_CARE,
			IMAGE_LAYOUT_UNDEFINED,
			IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		pass1->addAttachment({ color, 0 }, { depth, 1 });
		Subpass subpass{
			{color1, color2},
			{depth},
		};
		subpass->AddSubpass(subpass);

		subpass->addDependency();

		RenderPass pass1 = {
			attachments,
			subpass,
			dependency,
		};
		pass1->createframebuffer(width, height);

		m_Renderer->AddRenderingPipeline({
			pass1,

			});

		/////////////////////////////
	
        m_Pipeline = new GraphicsPipelineVulkan();

        VkShaderModule vertexModule = VulkanUtilities::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
        VkShaderModule fragmentModule = VulkanUtilities::CreateShaderModule(s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleFS.spv");
        m_Pipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
        m_Pipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);

        PipelineSpecs specs{};
        specs.AddModules(
                { "Shaders/Vulkan/Compiled/TriangleVS.spv",
                VERTEX_STAGE,},
                { "Shaders/Vulkan/Compiled/TriangleVS.spv",
                VERTEX_STAGE,}
        );

        std::vector<GraphicsPipelineVulkan::Binding> bindings;
        bindings.push_back({
            0,
            VK_VERTEX_INPUT_RATE_VERTEX,
            sizeof(Vertex),
            });
        std::vector<GraphicsPipelineVulkan::AttributeDescription> attributes;
        attributes.push_back({
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, Vertex::pos),
            });
        attributes.push_back({
            0,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, Vertex::normal),
            });
        attributes.push_back({
            0,
            2,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof(Vertex, Vertex::uv0),
            });
        attributes.push_back({
            0,
            3,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof(Vertex, Vertex::uv1),
            });
        attributes.push_back({
            0,
            4,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, Vertex::color),
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

		/////////////////////////////////
	}
}