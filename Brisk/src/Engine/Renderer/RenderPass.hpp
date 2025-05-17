#pragma once

#include "Texture.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	enum AttachmentType {
		SwapchainAttachment,
		Color,
		Depth,
	};

	struct RenderPassAttachment {
		//uint32_t pBinding;
		AttachmentType pAttachmentType;
		std::shared_ptr<Texture> pImage;
		//Core::Format pFormat;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		//virtual std::vector<RenderPassAttachment> GetAttachments() { return m_Attachments; }
		//int GetClearCount() const { return m_ClearCount; }
		int GetColorAttachmentCount() const { return m_ColorAttachmentCount; }

		static std::shared_ptr<RenderPass> Create();

	protected:
		bool m_HasDepth;
		int m_ColorAttachmentCount = 0;
		int m_ClearCount = 0;
	};
};
