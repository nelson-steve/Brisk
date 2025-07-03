#pragma once

#include "Engine/Renderer/Semaphore.hpp"

#include <volk.h>

namespace Brisk
{
    class SemaphoreVulkan : public Semaphore {
    public:
        virtual void Init() override;
        virtual void Release() override;

        VkSemaphore Get();
    private:
        VkSemaphore m_Semaphore;
    };
}