#pragma once

#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	static class SwapchainFactory {
	public:
		[[nodiscard]] static Swapchain* CreateSwapchain(WindowBase* win) {
			if (Engine::s_EngineInfo.API == EngineInfo::GraphicsAPI::Vulkan) {
				return new SwapchainVulkan(win);
			}
			else {
				BRISK_CORE_ERROR("Only Vulkan supported");
			}
		}
	};
}