#include "Fence.hpp"

namespace Brisk 
{
    std::shared_ptr<Fence> Fence::Create(){
        return std::make_shared<Fence>();
    }
}