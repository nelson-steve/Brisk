#include "Mutexes.hpp"

namespace Brisk {
	std::mutex g_GraphicsQueueMutex;
	std::mutex g_TransferCommandBufferMutex;
	std::mutex g_MainThreadCallbackMutex;
}