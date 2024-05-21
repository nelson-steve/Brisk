#include "PhysicalDevice.hpp"
#include "Core/Log.hpp"
#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"

#include <set>

namespace Brisk 
{
	PhysicalDevice::PhysicalDevice() {

	}

	void PhysicalDevice::Create(VkSurfaceKHR surface) {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(dynamic_cast<GraphicsDeviceVulkan*>(Engine::m_GPUDevice)->GetInstance(), &device_count, nullptr);
		if (device_count == 0) {
			BRISK_CORE_ERROR("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(dynamic_cast<GraphicsDeviceVulkan*>(Engine::m_GPUDevice)->GetInstance(), &device_count, devices.data());
		for (const auto& device : devices) {
			if (IsDeviceSuitable(device, surface)) {
				m_PhysicalDevice = device;
				break;
			}
		}
		if (m_PhysicalDevice == VK_NULL_HANDLE) {
			BRISK_CORE_ERROR("Failed to find a suitable GPU!");
		}

		//CreateQueueFamilies(m_PhysicalDevice, surface);
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> uniqueQueueFamilies = { 0, 0 };
		float queue_priority = 1.0f;
		for (uint32_t queue_family : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;
		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = static_cast<uint16_t>(queue_create_infos.size());
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.pEnabledFeatures = &device_features;
		std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		device_create_info.enabledExtensionCount = static_cast<uint16_t>(requiredExtensions.size());
		device_create_info.ppEnabledExtensionNames = requiredExtensions.data();

#if _DEBUG
		const std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
		};
		device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		device_create_info.ppEnabledLayerNames = validation_layers.data();
#else
		device_create_info.enabledLayerCount = 0;
#endif
		if (vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &m_Device) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create logical device!");
		}
		vkGetDeviceQueue(m_Device, 0, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, 0, 0, &m_PresentQueue);
	}

	void PhysicalDevice::CreateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				m_Indices.HasGraphicsSupport = true;
				m_Indices.GraphicsIndex = i;
			}
			 
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				m_Indices.HasPresentSupport = true;
				m_Indices.PresentIndex = i;
			}

			if (m_Indices.IsComplete()) {
				break;
			}

			i++;
		}

		BRISK_CORE_INFO("Present index:{}", m_Indices.HasPresentSupport);
		BRISK_CORE_INFO("Graphics index:{}", m_Indices.HasGraphicsSupport);
	}

	bool PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
		CreateQueueFamilies(device, surface);

		std::vector<const char*> extensions = dynamic_cast<GraphicsDeviceVulkan*>(Engine::m_GPUDevice)->GetRequiredExtenstions();
		bool extensionsSupported = false;
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			extensionsSupported = requiredExtensions.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		
		bool suitable = m_Indices.IsComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;

		return suitable;
	}
}