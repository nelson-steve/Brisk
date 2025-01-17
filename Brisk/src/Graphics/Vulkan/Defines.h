#pragma once

// INCLUDES
#include "Core/Log.hpp"
//--------------------
#define VK_NO_PROTOYPES
#include <vulkan/vulkan.hpp>
//--------------------------

#if _DEBUG
#define VK_LOG(status, message) if(status != VK_SUCCESS) { BRISK_APP_ERROR(message); }
#else
#define VK_LOG(status, message)
#endif
