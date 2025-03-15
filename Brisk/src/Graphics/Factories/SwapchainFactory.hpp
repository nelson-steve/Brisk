#pragma once

// INCLUDES
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"
//--------------------------------------------

namespace Brisk {
	class SwapchainFactory {
	public:
		[[nodiscard]] static std::shared_ptr<Swapchain> CreateSwapchain(std::shared_ptr<Window> window) {
			if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
				return std::make_shared<SwapchainVulkan>(window);
			}
			else {
				BRISK_CORE_ERROR("Only Vulkan supported");
			}
		}
	};
}