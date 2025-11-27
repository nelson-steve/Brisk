#include "SBT.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/SBTVulkan.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/SBTDirectX12.hpp"
#endif
namespace Brisk
{

	std::shared_ptr<SBT> SBT::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<SBTVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<SBTDirectX12>();
		}
#endif
	}
}