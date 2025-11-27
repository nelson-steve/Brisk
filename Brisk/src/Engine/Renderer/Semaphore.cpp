// INCLUDES
#include "Semaphore.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SemaphoreVulkan.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/SemaphoreDirectX12.hpp"
#endif
//-------------------------------------------

namespace Brisk 
{
    std::shared_ptr<Semaphore> Semaphore::Create(){
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<SemaphoreVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<SemaphoreDirectX12>();
		}
#endif
    }
}