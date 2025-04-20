#pragma once

#include "Texture.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	enum AttachmentType {
		Color,
		Depth,
	};

	struct RenderPassAttachment {
		uint32_t pBinding;
		AttachmentType pAttachmentType;
		std::shared_ptr<Texture> pImage;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(const std::vector<RenderPassAttachment>& inputs, const std::vector<RenderPassAttachment>& outputs) = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		static std::shared_ptr<RenderPass> Create();
	};
};
