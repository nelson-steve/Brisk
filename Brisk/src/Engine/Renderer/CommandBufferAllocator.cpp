#include "CommandBufferAllocator.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/CommandBufferAllocatorVulkan.hpp"
#include "Graphics/DirectX12/CommandBufferAllocatorDirectX12.hpp"

namespace Brisk
{
	std::shared_ptr<CommandBufferAllocator> CommandBufferAllocator::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<CommandBufferAllocatorVulkan>();
		}
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<CommandBufferAllocatorDirectX12>();
		}
	}
}