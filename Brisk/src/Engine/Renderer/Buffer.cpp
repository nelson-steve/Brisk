// INCLUDES
#include "pch.hpp"
#include "Buffer.hpp"
#include "Graphics/Vulkan/BufferVulkan.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/BufferDirectX12.hpp"
#endif
//----------------------------------------

namespace Brisk
{
	std::shared_ptr<Buffer> Buffer::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<BufferVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<BufferDirectX12>();
		}
#endif
	}
}