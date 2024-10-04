#include "Renderer.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk {
	std::unique_ptr<Renderer> Renderer::Create() {
		return std::make_unique<RendererVulkan>();
	}
}