#include "pch.hpp"
#include "GpuTIming.hpp"
#include "Graphics/Vulkan/GpuTimingVulkan.hpp"
#include "Engine/Engine.hpp"

namespace Brisk
{
	std::shared_ptr<GpuTiming> GpuTiming::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<GpuTimingVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<GpuTimingDirectX12>();
		}
#endif
	}
}