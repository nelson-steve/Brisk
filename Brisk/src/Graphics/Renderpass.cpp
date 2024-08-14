#include "RenderPass.hpp"
#include "Engine/Engine.hpp"
#include "Vulkan/RenderPassVulkan.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	RenderPass* RenderPass::Create() {
		switch (Engine::s_EngineInfo.API)
		{
		case EngineInfo::GraphicsAPI::Vulkan: {
			RenderPassVulkan* renderpass = new RenderPassVulkan();
			renderpass->Create(3);
			return renderpass;
		}
		case EngineInfo::GraphicsAPI::DirectX:
			BRISK_APP_ERROR("ERROR: Only Vulkan is supported currently");
			return nullptr;
		default:
			return nullptr;
		}
	}

	void RenderPass::Release(RenderPass* renderpass) {
		switch (Engine::s_EngineInfo.API)
		{
		case EngineInfo::GraphicsAPI::Vulkan:
			static_cast<RenderPassVulkan*>(renderpass)->Release();
			break;
		case EngineInfo::GraphicsAPI::DirectX:
			BRISK_APP_ERROR("ERROR: Only Vulkan is supported currently");
			break;
		}
	}
}