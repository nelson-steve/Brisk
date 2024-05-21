#pragma once

#include "Graphics/Swapchain.hpp"

namespace Brisk {
	class GPUDevice {
	public:
		[[nodiscard]] static GPUDevice* CreateDevice();

		virtual void Create() = 0;

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		virtual void Release() = 0;
	};
}