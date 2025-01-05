#pragma once

#include <memory>

namespace Brisk 
{
    class Semaphore {
    public:
        void Init();

        static std::shared_ptr<Semaphore> Create();
    };
}