#pragma once

#include "Engine/Renderer/Fence.hpp"

#include <Volk/volk.h>

namespace Brisk 
{
    class FenceVulkan : public Fence {
    public:
        virtual void Init() override;

        virtual void Wait() override;
        virtual void Reset() override;
    private:
        VkFence mFence;
    };
}