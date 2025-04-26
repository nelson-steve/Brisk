// INCLUDES
#include "Semaphore.hpp"
#include "Graphics/Vulkan/SemaphoreVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/DirectX12/SemaphoreDirectX12.hpp"
//-------------------------------------------

namespace Brisk 
{
    std::shared_ptr<Semaphore> Semaphore::Create(){
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<SemaphoreVulkan>();
		}
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<SemaphoreDirectX12>();
		}
    }
}