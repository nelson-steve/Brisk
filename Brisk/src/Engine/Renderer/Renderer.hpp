#pragma once

#include "Graphics/Swapchain.hpp"

namespace Brisk {
	class Renderer {
	public:
		virtual ~Renderer() {};

		virtual void Create() = 0;
		virtual void Release() = 0;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) = 0;

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;

		////const std::vector<VkFramebuffer> GetFramebuffers() const { return m_RenderPass->GetFramebuffers(); }
		//RenderPass* GetRenderPass() const { return m_RenderPass; }
		//const GraphicsPipelineVulkan* GetDefaultGraphicsPipeline() const { return m_DefaultGraphicsPipeline; }
	private:
		//BufferVulkan* m_VertexBuffer;
		//RenderPass* m_RenderPass;
		//GraphicsPipelineVulkan* m_DefaultGraphicsPipeline;
	};
}