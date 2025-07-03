#pragma once

// INCLUDES
#include "Core/Core.hpp"
//---------------------
#include <memory>
//---------------

namespace Brisk 
{
    class Fence {
        DEFINE_BASE_CLASS_CONSTRUCTOR(Fence)
    public:
        virtual void Init() = 0;
        virtual void Release() = 0;

        virtual void Wait() = 0;
        virtual void Reset() = 0;

        static std::shared_ptr<Fence> Create();
    };
}