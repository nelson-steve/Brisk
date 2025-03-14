// INCLUDES
#include "GpuAdapter.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<GpuAdapter> GpuAdapter::Create() {
		switch (Engine::s_EngineSettings.API)
		{
			case Engine::EngineSettings::GraphicsAPI::Vulkan:
				return std::make_shared<GpuAdapterVulkan>();
			case Engine::EngineSettings::GraphicsAPI::DirectX12:
				return std::make_shared<GpuAdapterDirectX12>();
		}
	}
}