#pragma once

#include <memory>

namespace Brisk 
{
    class Fence {
    public:
        void Init();

        void Wait();
        void Reset();

        static std::shared_ptr<Fence> Create();
    };
}