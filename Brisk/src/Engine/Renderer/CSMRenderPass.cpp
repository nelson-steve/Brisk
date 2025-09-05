// INCLUDES
#include "Graphics/Vulkan/CSMRenderPassVulkan.hpp"
#include "Engine/Engine.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/CSMRenderPassDirectX12.hpp"
#endif
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<CSMRenderPass> CSMRenderPass::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<CSMRenderPassVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<CSMRenderPassDirectX12>();
		}
#endif
	}
}