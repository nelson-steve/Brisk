#include "Semaphore.hpp"

namespace Brisk 
{
    void Semaphore::Create(){
        return std::make_shared<Semaphore>();
    }
}