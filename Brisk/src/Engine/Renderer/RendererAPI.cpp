// INCLUDES
#include "RendererAPI.hpp"
//-----------------------
#include "Graphics/Vulkan/RendererAPIVulkan.hpp"
#include "Graphics/Vulkan/ComputeAPIVulkan.hpp"
//---------------------------------------------

namespace Brisk 
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	RendererAPI* RendererAPI::Create() {
		return new RendererAPIVulkan();
	}

	ComputeAPI* ComputeAPI::Create() {
		return new ComputeAPIVulkan();
	}
}