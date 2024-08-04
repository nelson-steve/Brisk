#pragma once

#include <Volk/volk.h>

#include <vector>
#include <string>

namespace Brisk 
{
	struct QueueFamilyIndices {
		bool HasGraphicsSupport = false;
		bool HasPresentSupport = false;
		int32_t GraphicsIndex = -1;
		int32_t PresentIndex = -1;

		inline bool IsComplete() { return (HasGraphicsSupport && HasPresentSupport); }
		inline void Reset() { HasGraphicsSupport = false; HasPresentSupport = false; }
	};

	class PhysicalDevice {
	public:
		struct QueueInfo {
			enum QueueType {
				QUEUE_GRAPHICS_BIT,
				QUEUE_COMPUTE_BIT,
				QUEUE_TRANSFER_BIT,
				QUEUE_SPARSE_BINDING_BIT,
				QUEUE_PROTECTED_BIT,
				QUEUE_VIDEO_DECODE_BIT_KHR,
				QUEUE_VIDEO_ENCODE_BIT_KHR,
				QUEUE_OPTICAL_FLOW_BIT_NV,
			} m_QueueType;
			float Priority;
			uint32_t QueueIndex;
			uint32_t QueueFamilyIndex;
			uint32_t QueueCount;
			bool PresentSupport = false;
			std::vector<QueueType> SupportedQueueTypes;
		};
		enum Feature {
			PRESENTATION,
			ANISOTROPY,
		};
		struct Details {
		public:
			std::vector<Feature> RequiredFeatures;
			std::vector<QueueInfo::QueueType> RequiredQueueTypes;
			VkSurfaceKHR Surface = VK_NULL_HANDLE;
			VkBool32 SamplerAnisotropy = VK_TRUE;

		};
	public:
		void Create(const Details& details);
		void Release();
		void CreateQueueFamilies(VkPhysicalDevice device, const Details& details);
		bool IsDeviceSuitable(VkPhysicalDevice device, const Details& details);
		//QueueFamilyIndices GetQueueFamilies() { return m_Indices; }
		VkQueueFlags QueueTypeToVulkanType(QueueInfo::QueueType type);
		const std::string& QueueTypeToString(QueueInfo::QueueType type);
		void PrintQueueFlags(VkQueueFlags flags);
		void GetSupportedQueueTypes(VkQueueFlags flags, std::vector<QueueInfo::QueueType>& ref);
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		const std::vector<PhysicalDevice::QueueInfo> RetrieveCommonQueues();
		VkDevice GetDevice() { return m_Device; }
		const VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		const VkQueue GetPresentQueue() const { return m_PresentQueue; }
	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		//QueueFamilyIndices m_Indices;
		std::vector<QueueInfo> m_Queues;
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;
	};
}