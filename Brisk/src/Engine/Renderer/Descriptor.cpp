// INCLUDES
#include "Descriptor.hpp"
#include "Graphics/Vulkan/DescriptorLayoutVulkan.hpp"
#include "Engine/Engine.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/DescriptorLayoutDirectX12.hpp"
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

	void DescriptorLayout::SetDescriptorType(GpuDescriptorResourceType type) {
		m_Type = type;
	}

	GpuDescriptorResourceType DescriptorLayout::GetDescriptorType() {
		return m_Type;
	}

}