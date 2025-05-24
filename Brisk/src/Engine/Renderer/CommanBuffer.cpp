// INCLUDES
#include "CommandBuffer.hpp"
#include "Engine/Engine.hpp"

#include "Graphics/Vulkan/CommandBufferVulkan.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/CommandBufferDirectX12.hpp"
#endif
//-----------------------------------------------

namespace Brisk {
	std::shared_ptr<CommandBuffer> CommandBuffer::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<CommandBufferVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<CommandBufferDirectX12>();
		}
#endif
	}
}