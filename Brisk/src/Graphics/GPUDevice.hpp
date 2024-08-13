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

	class GPUContext {
	public:
		[[nodiscard]] static GPUContext* CreateContext();

		virtual void Create() = 0;

		virtual void Sync() = 0;
		virtual void WaitDeviceIdle() = 0;

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		virtual void Release() = 0;
	};
}