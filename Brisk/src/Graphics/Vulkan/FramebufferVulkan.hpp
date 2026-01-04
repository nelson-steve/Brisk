#pragma once

#include "Engine/Renderer/Framebuffer.hpp"

#include <volk.h>

namespace Brisk
{
	class FramebufferVulkan : public Framebuffer {
	public:
		virtual void Init(const FramebufferSpecs& specs) override;
		virtual void Destroy() override;

		VkFramebuffer GetFramebuffer() const { return m_Framebuffer; }
	private:
		VkFramebuffer m_Framebuffer;
	};
}