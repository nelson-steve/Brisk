// INCLUDES
#include "Queue.hpp"
#include "Graphics/Vulkan/QueueVulkan.hpp"
#include "Engine/Engine.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/QueueDirectX12.hpp"
#endif
//---------------------------------------

namespace Brisk
{
	std::shared_ptr<Queue> Queue::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<QueueVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<QueueDirectX12>();
		}
#endif
	}
}