#pragma once

// INCLUDES
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"
#include "Graphics/DirectX12/SwapchainDirectX12.hpp"
//--------------------------------------------

namespace Brisk {
	class SwapchainFactory {
	public:
		[[nodiscard]] static std::shared_ptr<Swapchain> CreateSwapchain(std::shared_ptr<Window> window) {
			if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
				return std::make_shared<SwapchainVulkan>(window);
			}
			else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
				return std::make_shared<SwapchainDirectX12>(window);
			}
		}
	};
}