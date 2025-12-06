#include "pch.hpp"
#include "CommandBufferAllocator.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/CommandBufferAllocatorVulkan.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/CommandBufferAllocatorDirectX12.hpp"
#endif

namespace Brisk
{
	std::shared_ptr<CommandBufferAllocator> CommandBufferAllocator::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<CommandBufferAllocatorVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<CommandBufferAllocatorDirectX12>();
		}
#endif
	}
}