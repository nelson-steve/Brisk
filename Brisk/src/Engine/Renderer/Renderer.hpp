#pragma once

#include "Graphics/GPUDevice.hpp"
#include "Graphics/Vulkan/GraphicsPipelineVulkan.hpp"
#include "Graphics/RenderPass.hpp"

namespace Brisk {
	class Renderer {
	public:
		void Initialize();
		void Release();

		void PreProcess();
		void Render();
		void PostProcess();

		//const std::vector<VkFramebuffer> GetFramebuffers() const { return m_RenderPass->GetFramebuffers(); }
		RenderPass* GetRenderPass() const { return m_RenderPass; }
		const GraphicsPipelineVulkan* GetDefaultGraphicsPipeline() const { return m_DefaultGraphicsPipeline; }
	private:
		RenderPass* m_RenderPass;
		GraphicsPipelineVulkan* m_DefaultGraphicsPipeline;
	};
}