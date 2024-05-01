#pragma once

#include "Core/Log.hpp"

#include <vulkan/vulkan.hpp>

#if _DEBUG
#define VK_LOG(status, message) if(status != VK_SUCCESS) { BRISK_APP_ERROR(message); }
#else
#define VK_LOG(status, message)
#endif
