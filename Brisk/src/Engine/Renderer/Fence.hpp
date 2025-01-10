#pragma once

// INCLUDES
#include <memory>
//---------------

namespace Brisk 
{
    class Fence {
    public:
        virtual void Init() = 0;

        virtual void Wait() = 0;
        virtual void Reset() = 0;

        static std::shared_ptr<Fence> Create();
    };
}