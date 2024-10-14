#include "RenderPass.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<RenderPass> RenderPass::Create() {
		return std::make_shared<RenderPassVulkan>();
	}
}