#include <stb_image.h>

#include <Volk/volk.h>

#include <string>

namespace Brisk {
	class TextureVulkan {
	public:
		void Create(const std::string& path);
	private:
		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkSampler m_Sampler;

		VkBuffer m_StagingBuffer;
		VkDeviceMemory m_StagingBufferMemory;
	};
}