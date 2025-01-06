#pragma once

#include "Engine/Renderer/Queue.hpp"

#include <Volk/volk.h>

namespace Brisk
{
    class QueueVulkan : public Queue {
    public:
        void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence);
		void Present();
    private:
        VkQueue mQueue;
    }
}