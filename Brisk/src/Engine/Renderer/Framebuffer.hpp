#pragma once

#include <cstdint>

namespace Brisk 
{
    class Framebuffer {
    public:
        Framebuffer(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}
        virtual ~Framebuffer() = default;

        virtual void Create() = 0;
        virtual void Destroy() = 0;
        virtual void Bind() = 0;

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
    };

}