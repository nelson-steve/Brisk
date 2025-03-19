// INCLUDES
#include "RendererAPI.hpp"
//-----------------------
#include "Graphics/Vulkan/RendererAPIVulkan.hpp"
#include "Graphics/Vulkan/ComputeAPIVulkan.hpp"
//---------------------------------------------

namespace Brisk 
{
	//RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	RendererAPI* RendererAPI::Create() {
		switch (Engine::s_EngineSettings.API)
		{
			case Engine::EngineSettings::GraphicsAPI::Vulkan:
				return new RendererAPIVulkan();
			//case Engine::EngineSettings::GraphicsAPI::DirectX12:
				//return new RendererAPIDirectX12();
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