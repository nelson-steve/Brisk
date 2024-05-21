#include "Graphics/Vulkan/GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	GPUDevice* GPUDevice::CreateDevice() {
		switch (Engine::s_EngineInfo.API)
		{
		case EngineInfo::GraphicsAPI::Vulkan:
			return new GraphicsDeviceVulkan();
			break;
		case EngineInfo::GraphicsAPI::DirectX:
			BRISK_APP_ERROR("ERROR: Only Vulkan is supported currently");
			return  nullptr;
			break;
		}
	}
}