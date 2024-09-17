#include "BufferVulkan.hpp"

#include <stb_image.h>
#include <Volk/volk.h>

#include <string>

namespace Brisk {
	class TextureVulkan {
	public:
		void Create(const std::string& path);
		VkImageView GetView() { return m_ImageView; }
		VkSampler GetSampler() { return m_Sampler; }
		VkImage Get() const { return m_Image; }
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	private:
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;
		uint32_t m_Width, m_Height;
		BufferVulkan* m_StagingBuffer;
	};
}