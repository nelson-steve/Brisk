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

	enum TargetType {
		Input,
		Output,
	};

	struct RenderPassAttachment {
		uint32_t pBinding;
		AttachmentType pAttachmentType;
		TargetType ptarget;
		std::shared_ptr<Texture> pImage;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void AddInputAttachment(RenderPassAttachment attachment) = 0;
		virtual void AddOutputAttachments(const std::vector<RenderPassAttachment>& attachments) = 0;
		virtual void Init() = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		static std::shared_ptr<RenderPass> Create();
	};
};
