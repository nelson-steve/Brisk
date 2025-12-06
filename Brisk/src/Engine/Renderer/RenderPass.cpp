// INCLUDES
#include "pch.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"
#include "Engine/Engine.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/RenderPassDirectX12.hpp"
#endif
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<RenderPass> RenderPass::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<RenderPassVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<RenderPassDirectX12>();
		}
#endif
	}
}