#pragma once

#include "Engine/Renderer/Framebuffer.hpp"

namespace Brisk 
{
	class FramebufferVulkan : public Framebuffer {
    public:
        virtual void Init(const FramebufferSpecs& specs) override;
        virtual void Destroy() override;

        virtual void Bind() override;

    private:
        VkFramebuffer m_Framebuffer;
	};
}