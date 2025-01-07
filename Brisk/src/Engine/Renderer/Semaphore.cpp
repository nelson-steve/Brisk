#include "Semaphore.hpp"

namespace Brisk 
{
    std::shared_ptr<Semaphore> Semaphore::Create(){
        return std::make_shared<Semaphore>();
    }
}