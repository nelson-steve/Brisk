#pragma once

#include <memory>

namespace Brisk 
{
    class Semaphore {
    public:
        virtual void Init() = 0;

        static std::shared_ptr<Semaphore> Create();
    };
}