#pragma once

#include <vulkan/vulkan.hpp>

#if _DEBUG
#define ERROR_FATAL(message) {printf(message);exit(-1);}
#else
#define ERROR_FATAL(message) {exit(-1);}
#endif
