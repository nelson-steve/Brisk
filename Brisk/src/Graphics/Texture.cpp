#include "Texture.hpp"
#include "Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<Texture> Texture::Create() {
		return std::make_shared<TextureVulkan>();
	}
}