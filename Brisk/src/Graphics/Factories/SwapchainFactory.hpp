#pragma once

// INCLUDES
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/SwapchainDirectX12.hpp"
#endif
//--------------------------------------------

namespace Brisk {
	class SwapchainFactory {
	public:
		[[nodiscard]] static std::shared_ptr<Swapchain> CreateSwapchain() {
			if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
				return std::make_shared<SwapchainVulkan>();
			}
#ifdef BRISK_ENABLE_DIRECTX12
			else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
				return std::make_shared<SwapchainDirectX12>();
			}
#endif
		}
	};
}