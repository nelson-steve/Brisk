#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

namespace Brisk 
{
	class RendererFactory {
	public:
		[[nodiscard]] static Renderer* CreateRenderer() {
			if (Engine::s_EngineInfo.API == EngineInfo::GraphicsAPI::Vulkan) {
				return new RendererVulkan();
			}
			else {
				BRISK_CORE_ERROR("Only Vulkan supported");
			}
		}
	};
}