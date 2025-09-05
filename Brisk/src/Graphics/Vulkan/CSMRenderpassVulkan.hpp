#pragma once

// INCLUDES
#include "Engine/Renderer/CSMRenderPass.hpp"
//------------------------
#include <volk.h>
//----------------

namespace Brisk
{
	class CSMRenderPassVulkan : public CSMRenderPass {
	public:
		virtual void Init(const std::vector<std::shared_ptr<Texture>> framebuffers) override;
		virtual void Release() override;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) override;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) override;

		virtual VkRenderPass GetRenderPass() { return m_RenderPass; }
	private:
		VkRenderPass m_RenderPass;
		std::vector<VkFramebuffer> m_Framebuffers;
	};
}