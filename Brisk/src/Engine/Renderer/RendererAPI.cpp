// INCLUDES
#include "pch.hpp"
#include "RendererAPI.hpp"
//-----------------------
#include "Graphics/Vulkan/RendererAPIVulkan.hpp"
#include "Graphics/Vulkan/ComputeAPIVulkan.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/RendererAPIDirectX12.hpp"
#endif
//---------------------------------------------

namespace Brisk 
{
	RendererAPI* RendererAPI::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return new RendererAPIVulkan();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return new RendererAPIDirectX12();
		}
#endif
	}

	ComputeAPI* ComputeAPI::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return new ComputeAPIVulkan();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			//return new ComputeAPIDirectX12();
			return nullptr;
		}
#endif
	}
}