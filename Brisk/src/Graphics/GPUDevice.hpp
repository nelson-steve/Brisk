#pragma once

#include "Graphics/Swapchain.hpp"

#include <vector>

namespace Brisk {
	enum ShaderType {
		Vertex,
		Fragment,
		Compute,
		Geometry
	};

	struct ShaderInfo {
		std::string Path;
		ShaderType Type;
	};

	class GPUDevice {
	public:
		[[nodiscard]] static GPUDevice* CreateDevice();

		virtual void Create() = 0;

		virtual void SetupGraphicsPipeline(std::vector<ShaderInfo> shaders) = 0;
		virtual void ReleaseGraphicsPipeline() = 0;

		virtual void Draw() = 0;
		virtual void WaitDeviceIdle() = 0;

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		virtual void Release() = 0;
	};
}