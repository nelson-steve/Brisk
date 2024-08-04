#pragma once

#include "Graphics/GPUDevice.hpp"
#include "Graphics/Vulkan/GraphicsPipelineVulkan.hpp"
#include "Graphics/Vulkan/RenderpassVulkan.hpp"

namespace Brisk {
	class Renderer {
	public:
		void Initialize();
		void Release();

		void PreProcess();
		void Render();
		void PostProcess();

		const std::vector<VkFramebuffer> GetFramebuffers() const { return m_Renderpass->GetFramebuffers(); }
		const VkRenderPass GetRenderpass() const { return m_Renderpass->GetRenderPass(); }
		const GraphicsPipelineVulkan* GetDefaultGraphicsPipeline() const { return m_DefaultGraphicsPipeline; }
	private:
		RenderpassVulkan* m_Renderpass;
		GraphicsPipelineVulkan* m_DefaultGraphicsPipeline;
	};
}