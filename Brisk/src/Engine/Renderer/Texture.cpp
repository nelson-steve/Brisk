#include "Engine/Renderer/Texture.hpp"
#include "Graphics/Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<Texture> Texture::Create() {
		return std::make_shared<TextureVulkan>();
	}
}