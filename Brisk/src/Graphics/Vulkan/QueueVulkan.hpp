#pragma once

#include "Engine/Renderer/Queue.hpp"

#include <volk.h>

namespace Brisk
{
    class QueueVulkan : public Queue {
    public:
        virtual void Submit(SubmitInfo submitInfo, std::shared_ptr<Fence> fence) override;
        virtual void Present(Queue::PresentInfo info) override;

        virtual void Init(Queue::QueueType type) override;
    private:
        VkQueue m_Queue;
    };
}