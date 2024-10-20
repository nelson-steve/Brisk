#include "RendererAPI.hpp"

#include "Graphics/Vulkan/RendererAPIVulkan.hpp"

namespace Brisk 
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	RendererAPI* RendererAPI::Create() {
		return new RendererAPIVulkan();
	}
}