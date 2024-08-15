#pragma once

#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/RenderPassVulkan.hpp"
#include "Core/Log.hpp"

namespace Brisk {
	static class RenderPassFactory {
	public:
		[[nodiscard]] static RenderPass* CreateRenderPass() {
			if (Engine::s_EngineInfo.API == EngineInfo::GraphicsAPI::Vulkan) {
				return new RenderPassVulkan();
			}
			else {
				BRISK_CORE_ERROR("Only Vulkan supported");
			}
		}
	};
}