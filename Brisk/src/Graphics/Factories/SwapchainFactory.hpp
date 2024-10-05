#pragma once

#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	class SwapchainFactory {
	public:
		[[nodiscard]] static Swapchain* CreateSwapchain(std::shared_ptr<Window> window) {
			if (Engine::s_EngineInfo.API == EngineInfo::GraphicsAPI::Vulkan) {
				return new SwapchainVulkan(window);
			}
			else {
				BRISK_CORE_ERROR("Only Vulkan supported");
			}
		}
	};
}