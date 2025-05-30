#pragma once

#include "BufferVulkan.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Application.hpp"

#include <stb_image.h>
#include <volk.h>

#include <string>

namespace Brisk
{
	class TextureVulkan : public Texture
	{
	public:
		VkImageView GetView() { return m_ImageView; }
		VkImage		GetImage() { return m_Image; }
		VkSampler	GetSampler() { return m_Sampler; }
		VkFormat	GetFormat() { return m_Format; }
		VkDescriptorImageInfo GetDescriptor() const { return m_Descriptor; }
		const VkDescriptorImageInfo* GetDescriptor() { return &m_Descriptor; }


		virtual void TransitionImageLayout(std::shared_ptr<CommandBuffer> cmd, std::vector<ImageBarrierParams> params) override;
		virtual void CopyImage(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> src, std::shared_ptr<Texture> dest, uint32_t width, uint32_t height) override;

		virtual uint32_t GetWidth() const { return m_Specs.p_Width; }
		virtual uint32_t GetHeight() const { return m_Specs.p_Height; }
		virtual void Resize() override { assert(false); }

		virtual void Init(const TextureSpecification& specs) override;
		virtual void Init(const std::string &path) override;
		virtual void Init(tinygltf::Image image, TextureSampler sampler) override;
	private:
		VkDescriptorImageInfo m_Descriptor;
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		// Just a workaround to only use 1 sampler
		static VkSampler m_Sampler;
		VkFormat m_Format;

		VkDevice m_DeviceCached;
	};
}