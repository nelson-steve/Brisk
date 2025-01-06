#pragma once

#include "Engine/Renderer/Semaphore.hpp"

#include <Volk/volk.h>

namespace Brisk
{
    class SemaphoreVulkan : public Semaphore {
    public:
        virtual void Init() override;

        VkSemaphore Get() const;
    private:
        VkSemaphore mSemaphore;
    };
}