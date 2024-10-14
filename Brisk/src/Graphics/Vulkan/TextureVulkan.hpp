#pragma once

#include "BufferVulkan.hpp"
#include "Graphics/BriskTexture.hpp"
#include "Engine/Application.hpp"

#include <stb_image.h>
#include <Volk/volk.h>

#include <string>

namespace Brisk 
{
	class TextureVulkan : public BriskTexture {
	public:
		VkImageView GetView() { return m_ImageView; }
		VkImage GetImage() { return m_Image; }
		VkSampler GetSampler() { return m_Sampler; }
		VkImage Get() const { return m_Image; }
		
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual void Resize() override {}

		void Create();
		//void Create(int width, int height);
		void Create(int width, int height, Format format, Type type);
		void Create(const std::string& path);
	private:

	private:
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;
	};
}