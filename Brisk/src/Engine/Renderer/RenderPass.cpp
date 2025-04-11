// INCLUDES
#include "Graphics/Vulkan/RenderPassVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/DirectX12/RenderPassDirectX12.hpp"
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<RenderPass> RenderPass::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<RenderPassVulkan>();
		}
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			//return std::make_shared<RenderPassDirectX12>();
			return nullptr;
		}
	}
}