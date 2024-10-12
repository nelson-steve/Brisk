#include "RenderPass.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<RenderPass> RenderPass::Create(const RenderPassSpecs& specs) {
		return std::make_shared<RenderPassVulkan>();
	}
}