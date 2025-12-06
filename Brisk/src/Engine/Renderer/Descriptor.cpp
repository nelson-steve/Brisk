// INCLUDES
#include "pch.hpp"
#include "Descriptor.hpp"
#include "Graphics/Vulkan/DescriptorLayoutVulkan.hpp"
#include "Engine/Engine.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#endif
//---------------------------------------------------

namespace Brisk
{
	std::shared_ptr<DescriptorLayout> DescriptorLayout::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<DescriptorLayoutVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			//return std::make_shared<DescriptorLayoutDirectX12>();
		}
#endif
	}
}