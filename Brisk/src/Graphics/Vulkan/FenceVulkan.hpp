#pragma once

#include "Engine/Renderer/Fence.hpp"

#include <Volk/volk.h>

namespace Brisk 
{
    class FenceVulkan : public Fence {
    public:
        virtual void Init() = 0;

        virtual void Wait() = 0;
        virtual void Reset() = 0;
    private:
        VkFence m_Fence;
    };
}