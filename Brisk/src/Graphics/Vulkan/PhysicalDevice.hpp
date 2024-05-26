#pragma once

#include <Volk/volk.h>

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
		PhysicalDevice();
		void Create(VkSurfaceKHR surface);
		void Release();
		void CreateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		QueueFamilyIndices GetQueueFamilies() { return m_Indices; }
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice GetDevice() { return m_Device; }
	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		QueueFamilyIndices m_Indices;
		VkQueue m_PresentQueue;
		VkQueue m_GraphicsQueue;
	};
}