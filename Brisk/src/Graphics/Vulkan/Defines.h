#pragma once

#include <vulkan/vulkan.hpp>

#if _DEBUG
#define VK_LOG(status, message) if(status != VK_SUCCESS) {printf(message);}
#else
#define VK_LOG(status, message) if(status != VK_SUCCESS) {printf(message);}
#endif
