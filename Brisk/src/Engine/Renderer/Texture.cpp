#include "Engine/Renderer/Texture.hpp"
#include "Graphics/Vulkan/TextureVulkan.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/Vulkan/TextureDirectX12.hpp"
#endif

namespace Brisk
{
	std::shared_ptr<Texture> Texture::Create() {
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<TextureVulkan>();
		}
#ifdef BRISK_ENABLE_DIRECTX12
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<TextureDirectX12>();
		}
#endif
	}
}