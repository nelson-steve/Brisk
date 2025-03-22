// INCLUDES
#include "RendererAPI.hpp"
//-----------------------
#include "Graphics/Vulkan/RendererAPIVulkan.hpp"
#include "Graphics/Vulkan/ComputeAPIVulkan.hpp"
#include "Graphics/DirectX12/RendererAPIDirectX12.hpp"
//---------------------------------------------

namespace Brisk 
{
	RendererAPI* RendererAPI::Create() {
		switch (Engine::s_EngineSettings.API)
		{
			case Engine::EngineSettings::GraphicsAPI::Vulkan:
				return new RendererAPIVulkan();
			case Engine::EngineSettings::GraphicsAPI::DirectX12:
				return new RendererAPIDirectX12();
		}
	}

	ComputeAPI* ComputeAPI::Create() {
		switch (Engine::s_EngineSettings.API)
		{
			case Engine::EngineSettings::GraphicsAPI::Vulkan:
				return new ComputeAPIVulkan();
			//case Engine::EngineSettings::GraphicsAPI::DirectX12:
				//return new ComputeAPIDirectX12();
		}
	}
}