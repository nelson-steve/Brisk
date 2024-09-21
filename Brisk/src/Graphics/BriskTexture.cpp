#include "BriskTexture.hpp"
#include "Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	BriskTexture* BriskTexture::Create() {
		TextureVulkan* texture = new TextureVulkan();
		texture->Create();
		return texture;
	}

	BriskTexture* BriskTexture::Create(int width, int height) {
		//TextureVulkan* texture = new TextureVulkan();
		//texture->Create(width, height);
		//return texture;
		return nullptr;
	}

	BriskTexture* BriskTexture::Create(int width, int height, Format format, Type type) {
		TextureVulkan* texture = new TextureVulkan();
		texture->Create(width, height, format, type);
		return texture;
	}

	BriskTexture* BriskTexture::Create(std::string path) {
		TextureVulkan* texture = new TextureVulkan();
		texture->Create(path);
		return texture;
	}
}