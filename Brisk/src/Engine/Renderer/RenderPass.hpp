#pragma once

// INCLUDES
#include "CommandBuffer.hpp"
#include "Swapchain.hpp"
#include "RHI.hpp"
#include "Engine/Renderer/Texture.hpp"
//---------------
#include <cstdint>
#include <vector>
#include <memory>
//--------------

namespace Brisk 
{
    class RenderPass {
    public:
        virtual void Execute() = 0;
        virtual ~RenderPass() = default;
    };
};
