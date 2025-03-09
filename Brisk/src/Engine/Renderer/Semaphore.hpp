#pragma once

// INCLUDES
#include "Core/Core.hpp"
//--------------
#include <memory>
//--------------

namespace Brisk 
{
    class Semaphore {
        DEFINE_BASE_CLASS_CONSTRUCTOR(Semaphore)
    public:
        virtual void Init() = 0;

        static std::shared_ptr<Semaphore> Create();
    };
}