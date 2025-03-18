#pragma once

// INCLUDES
#include "RenderPass.hpp"
#include "Engine/Renderer/Texture.hpp"
//-----------------------------
#include <cstdint>
#include <memory>
#include <vector>
//---------------

namespace Brisk 
{
    class Framebuffer {
        DEFINE_BASE_CLASS_CONSTRUCTOR(Framebuffer)
    public:
        struct FramebufferSpecs {
            uint32_t pWidth;
            uint32_t pHeight;
            uint32_t pLayers;
            std::shared_ptr<RenderPass> pRenderPass;
            std::vector<Texture> pAttachments;
        };

    public:
        virtual void Init(const FramebufferSpecs& specs) = 0;
        virtual void Destroy() = 0;

        virtual void Bind() = 0;

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        static std::shared_ptr<Framebuffer> Create(const FramebufferSpecs& specs);
    protected:
        uint32_t m_Width;
        uint32_t m_Height;
    };
}