#pragma once

#include "Texture.hpp"

#include <memory>

namespace Brisk 
{
	enum AttachmentType {
		Color,
		Depth,
	};

	struct RenderPassAttachment {
		uint32_t pBinding;
		//Core::Format pFormat;
		AttachmentType pAttachmentType;
		std::shared_ptr<Texture> pImage;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void AddInputAttachment(RenderPassAttachment attachment) = 0;
		virtual void AddOutputAttachment(RenderPassAttachment attachment) = 0;
		virtual void Init() = 0;

		static std::shared_ptr<RenderPass> Create();
	};
};
