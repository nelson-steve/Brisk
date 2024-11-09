#include "Texture.hpp"
#include "Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	std::shared_ptr<Texture> Texture::Create() {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create();
		return texture;
	}

	std::shared_ptr<Texture> Texture::Create(int width, int height) {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create(width, height);
		return texture;
	}

	std::shared_ptr<Texture> Texture::Create(int width, int height, Format format, Type type) {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create(width, height, format, type);
		return texture;
	}

	std::shared_ptr<Texture> Texture::Create(tinygltf::Image image, TextureSampler sampler) {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create(image, sampler);
		return texture;
	}

	std::shared_ptr<Texture> Texture::Create(std::string path) {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create(path);
		return texture;
	}

	std::shared_ptr<Texture> Texture::Create(std::string path) {
		std::shared_ptr<Texture> texture = std::make_shared<TextureVulkan>();
		texture->Create(path);
		return texture;
	}
}