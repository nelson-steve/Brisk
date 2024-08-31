#pragma once

#include <Volk/volk.h>

#include <string>
#include <vector>

namespace Brisk 
{
	class GraphicsPipelineVulkan {
	public:
        struct Binding {
            uint32_t BindingIndex;
            VkVertexInputRate InputRate;
            uint32_t Stride;
        };

        struct AttributeDescription {
            uint32_t BindingIndex;
            uint32_t Location;
            VkFormat Format;
            uint32_t Offset;
        };

	public:
		GraphicsPipelineVulkan();
		void CreatePipeline(VkRenderPass renderpass);
		void Release();

        void CreateShaderStage(VkShaderModule module, VkShaderStageFlagBits stage);
        void CreateVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
        void CreateInputAssembly(bool primitiveRestartEnable, VkPrimitiveTopology topology);
        void CreateViewportState(uint32_t count, uint32_t scissorCount);
        void CreateRasterizer(bool depthClamp, bool discard, VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode, VkFrontFace frontFace, bool depthBias);
        void CreateMultiSampling(bool sampleShading, VkSampleCountFlagBits samples);
        void CreateDepthStencil(bool depthTest, bool depthWrite, VkCompareOp, bool depthBoundsTest, bool stencilTest);
        void CrateColorBlending(const std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments, bool isLogicOp, VkLogicOp logicOp);
        void CreateDynamicState(const std::vector<VkDynamicState>& dynamicStates);
        void CreatePipelineLayout(uint32_t layoutCount, uint32_t pushConstantRangeCount);

		const VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
	private:
        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
        VkPipelineVertexInputStateCreateInfo m_VertexInputInfo;
        VkPipelineInputAssemblyStateCreateInfo m_InputAssembly;
        VkPipelineViewportStateCreateInfo m_ViewportState;
        VkPipelineRasterizationStateCreateInfo m_Rasterizer;
        VkPipelineMultisampleStateCreateInfo m_Multisampling;
        VkPipelineDepthStencilStateCreateInfo m_DepthStencil;
        VkPipelineColorBlendStateCreateInfo m_ColorBlending;
        VkPipelineDynamicStateCreateInfo m_DynamicState;
        VkPipelineLayoutCreateInfo m_PipelineLayoutInfo;

        std::vector<VkShaderModule> m_Modules;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
	};
}