#pragma once

#include <mutex>

namespace Brisk {
	extern std::mutex g_GraphicsQueueMutex;
	extern std::mutex g_TransferCommandBufferMutex;
	extern std::mutex g_MainThreadCallbackMutex;
}
