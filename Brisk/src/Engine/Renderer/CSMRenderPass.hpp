#pragma once

// INCLUDES
#include "Core/Core.hpp"
#include "RenderPass.hpp"
//------------------------
#include <volk.h>
//----------------

namespace Brisk 
{
	class CSMRenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(CSMRenderPass);
	public:
		virtual void Init(const std::vector<std::shared_ptr<Texture>> framebuffers) = 0;
		virtual void Release() = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t framebuffer) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		static std::shared_ptr<CSMRenderPass> Create();
	};
}