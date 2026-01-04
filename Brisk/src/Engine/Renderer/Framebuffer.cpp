// INCLUDES
#include "pch.hpp"
#include "Framebuffer.hpp"
#include "Graphics/Vulkan/FramebufferVulkan.hpp"
#include "Engine/Engine.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/FramebufferDirectX12.hpp"
#endif
//----------------------------------------

namespace Brisk
{
	std::shared_ptr<Framebuffer> Framebuffer::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<FramebufferVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<FramebufferDirectX12>();
		}
#endif
	}
}