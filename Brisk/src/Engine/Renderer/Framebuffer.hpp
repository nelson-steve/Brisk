#pragma once

// INCLUDES
#include "RenderPass.hpp"
#include "Engine/Renderer/Texture.hpp"
//-----------------------------
#include <memory>
#include <vector>
//---------------

namespace Brisk 
{
    class Framebuffer {
        DEFINE_BASE_CLASS_CONSTRUCTOR(Framebuffer)
    public:
        struct FramebufferSpecs {
            uint32_t p_Width;
            uint32_t p_Height;
            std::shared_ptr<RenderPass> p_RenderPass;
            int32_t swapchainIndex = -1;
            std::vector<std::shared_ptr<Texture>> p_Attachments;
        };

    public:
        virtual void Init(const FramebufferSpecs& specs) = 0;
        virtual void Destroy() = 0;

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        static std::shared_ptr<Framebuffer> Create();
    protected:
        uint32_t m_Width;
        uint32_t m_Height;
    };
}