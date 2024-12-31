#pragma once

#include "BufferVulkan.hpp"
#include "Graphics/Texture.hpp"
#include "Engine/Application.hpp"

#include <stb_image.h>
#include <Volk/volk.h>

#include <string>

namespace Brisk
{
	class TextureVulkan : public Texture
	{
	public:
		VkImageView GetView() { return m_ImageView; }
		VkImage GetImage() { return m_Image; }
		VkSampler GetSampler() { return m_Sampler; }
		VkImage Get() const { return m_Image; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void Resize() override {}

		VkDescriptorImageInfo GetDescriptor() const { return m_Descriptor; }
		const VkDescriptorImageInfo* GetDescriptor() { return &m_Descriptor; }

		// virtual void Init(int width, int height) override;
		virtual void Init() override;
		virtual void Init(int width, int height, Core::Format format, Type type) override;
		virtual void Init(tinygltf::Image image, TextureSampler sampler) override;
		virtual void Init(const std::string &path) override;

		// void Create();
		////void Create(int width, int height);
		// void Create(int width, int height, Format format, Type type);
		// void Create(const std::string& path);
	private:
		VkDescriptorImageInfo m_Descriptor;
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;
	};
}