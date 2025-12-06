// INCLUDES
#include "pch.hpp"
#include "GpuAdapter.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"
#endif
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<GpuAdapter> GpuAdapter::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<GpuAdapterVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<GpuAdapterDirectX12>();
		}
#endif
	}
}