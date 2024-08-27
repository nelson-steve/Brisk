#include "GPUDeviceVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"

#include <set>

namespace Brisk 
{
	void GPUDeviceVulkan::Create(const Details& details) {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(GPUContextVulkan::s_Instance, &device_count, nullptr);
		if (device_count == 0) {
			BRISK_CORE_ERROR("Failed to find GPUs with Vulkan support!");
			return;
		}
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(GPUContextVulkan::s_Instance, &device_count, devices.data());
		for (const auto& device : devices) {
			if (IsDeviceSuitable(device, details)) {
				m_PhysicalDevice = device;
				break;
			}
		}
		if (m_PhysicalDevice == VK_NULL_HANDLE) {
			BRISK_CORE_ERROR("Failed to find a suitable GPU!");
			return;
		}

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		//for (const QueueInfo& q : RetrieveCommonQueues()) {
		////for (const QueueInfo& q : m_Queues) {
		//	VkDeviceQueueCreateInfo queueCreateInfo{};
		//	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//	queueCreateInfo.queueFamilyIndex = q.QueueFamilyIndex;
		//	queueCreateInfo.queueCount = q.QueueCount;
		//	std::cout << "priorit: " << q.Priority << std::endl;
		//	queueCreateInfo.pQueuePriorities = m_QueueFamiliesPriorities[q.QueueFamilyIndex].data();
		//	queueCreateInfos.push_back(queueCreateInfo);
		//}

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;
		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = static_cast<uint16_t>(queueCreateInfos.size());
		device_create_info.pQueueCreateInfos = queueCreateInfos.data();
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

		//Queue* q1 = new Queue();
		//q1->Info = m_Queues[0];
		//Queue* q2 = new Queue();
		//q2->Info = m_Queues[0];
		//vkGetDeviceQueue(m_Device, q1->Info.QueueFamilyIndex, m_Queues[0].QueueIndex, &q1->Queue_);
		//vkGetDeviceQueue(m_Device, q2->Info.QueueFamilyIndex, m_Queues[0].QueueIndex, &q2->Queue_);
		//m_GraphicsQueue = q1;
		//m_PresentQueue = q2;
	}

	void GPUDeviceVulkan::Release() {
		delete m_PresentQueue;
		delete m_GraphicsQueue;
		vkDestroyDevice(m_Device, nullptr);
	}

	//const std::vector<GPUDeviceVulkan::QueueInfo> GPUDeviceVulkan::RetrieveCommonQueues() {
	//	std::vector<GPUDeviceVulkan::QueueInfo> queues;
	//	for (const auto& queueToAdd : m_Queues) {
	//		bool shouldAdd = true;
	//		for (const auto& queue : queues) {
	//			if (queue.QueueFamilyIndex == queueToAdd.QueueFamilyIndex) {
	//				shouldAdd = false;
	//				break;
	//			}
	//		}
	//		if(shouldAdd)
	//			queues.push_back(queueToAdd);
	//	}
	//
	//	return queues;
	//}

	const std::string& GPUDeviceVulkan::QueueTypeToString(QueueInfo::QueueType type) {
		switch (type)
		{
		case QueueInfo::QueueType::QUEUE_GRAPHICS_BIT:
			return "VK_QUEUE_GRAPHICS_BIT";
		case QueueInfo::QueueType::QUEUE_COMPUTE_BIT:
			return "VK_QUEUE_COMPUTE_BIT";
		case QueueInfo::QueueType::QUEUE_TRANSFER_BIT:
			return "VK_QUEUE_TRANSFER_BIT";
		case QueueInfo::QueueType::QUEUE_SPARSE_BINDING_BIT:
			return "VK_QUEUE_SPARSE_BINDING_BIT";
		case QueueInfo::QueueType::QUEUE_PROTECTED_BIT:
			return "VK_QUEUE_PROTECTED_BIT";
		case QueueInfo::QueueType::QUEUE_VIDEO_DECODE_BIT_KHR:
			return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
		case QueueInfo::QueueType::QUEUE_VIDEO_ENCODE_BIT_KHR:
			return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
		case QueueInfo::QueueType::QUEUE_OPTICAL_FLOW_BIT_NV:
			return "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
		default:
			// Error
			return "VK_QUEUE_FLAG_BITS_MAX_ENUM";
		}
	}

	void GPUDeviceVulkan::PrintQueueFlags(VkQueueFlags flags) {
		if (flags & VK_QUEUE_GRAPHICS_BIT)
			std::cout << "VK_QUEUE_GRAPHICS_BIT" << std::endl;
		if (flags & VK_QUEUE_COMPUTE_BIT)
			std::cout << "VK_QUEUE_COMPUTE_BIT" << std::endl;
		if (flags & VK_QUEUE_TRANSFER_BIT)
			std::cout << "VK_QUEUE_TRANSFER_BIT" << std::endl;
		if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
			std::cout << "VK_QUEUE_SPARSE_BINDING_BIT" << std::endl;
		if (flags & VK_QUEUE_PROTECTED_BIT)
			std::cout << "VK_QUEUE_PROTECTED_BIT" << std::endl;
		if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			std::cout << "VK_QUEUE_VIDEO_DECODE_BIT_KHR" << std::endl;
		if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			std::cout << "VK_QUEUE_VIDEO_ENCODE_BIT_KHR" << std::endl;
		if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
			std::cout << "VK_QUEUE_OPTICAL_FLOW_BIT_NV" << std::endl;
	}

	void GPUDeviceVulkan::GetSupportedQueueTypes(VkQueueFlags flags, std::vector<QueueInfo::QueueType>& ref) {
		if (flags & VK_QUEUE_GRAPHICS_BIT)
			ref.push_back(QueueInfo::QUEUE_GRAPHICS_BIT);
		if (flags & VK_QUEUE_COMPUTE_BIT)
			ref.push_back(QueueInfo::QUEUE_COMPUTE_BIT);
		if (flags & VK_QUEUE_TRANSFER_BIT)
			ref.push_back(QueueInfo::QUEUE_TRANSFER_BIT);
		if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
			ref.push_back(QueueInfo::QUEUE_SPARSE_BINDING_BIT);
		if (flags & VK_QUEUE_PROTECTED_BIT)
			ref.push_back(QueueInfo::QUEUE_PROTECTED_BIT);
		if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			ref.push_back(QueueInfo::QUEUE_VIDEO_DECODE_BIT_KHR);
		if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			ref.push_back(QueueInfo::QUEUE_VIDEO_ENCODE_BIT_KHR);
		if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
			ref.push_back(QueueInfo::QUEUE_OPTICAL_FLOW_BIT_NV);
	}

	VkQueueFlags GPUDeviceVulkan::QueueTypeToVulkanType(QueueInfo::QueueType type) {
		switch (type)
		{
			case QueueInfo::QueueType::QUEUE_GRAPHICS_BIT:
				return VK_QUEUE_GRAPHICS_BIT;
			case QueueInfo::QueueType::QUEUE_COMPUTE_BIT:
				return VK_QUEUE_COMPUTE_BIT;
			case QueueInfo::QueueType::QUEUE_TRANSFER_BIT:
				return VK_QUEUE_TRANSFER_BIT;
			case QueueInfo::QueueType::QUEUE_SPARSE_BINDING_BIT:
				return VK_QUEUE_SPARSE_BINDING_BIT;
			case QueueInfo::QueueType::QUEUE_PROTECTED_BIT:
				return VK_QUEUE_PROTECTED_BIT;
			case QueueInfo::QueueType::QUEUE_VIDEO_DECODE_BIT_KHR:
				return VK_QUEUE_VIDEO_DECODE_BIT_KHR;
			case QueueInfo::QueueType::QUEUE_VIDEO_ENCODE_BIT_KHR:
				return VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
			case QueueInfo::QueueType::QUEUE_OPTICAL_FLOW_BIT_NV:
				return VK_QUEUE_OPTICAL_FLOW_BIT_NV;
			default:
				// Error
				return VK_QUEUE_FLAG_BITS_MAX_ENUM;
		}
	}

	bool GPUDeviceVulkan::CreateQueueFamilies(VkPhysicalDevice device, const Details& details) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		bool requiredQueueTypesExist = false;
		m_QueueFamiliesPriorities.resize(queueFamilies.size());
		for (const auto& queueFamily : queueFamilies) {
			bool featuresExist = false;
			QueueInfo queue;
			queue.Priority = 0.5f;
			queue.QueueFamilyIndex = i;
			queue.QueueCount = queueFamily.queueCount;
			std::cout << "Queue family: " << i << std::endl;
			PrintQueueFlags(queueFamily.queueFlags);
			GetSupportedQueueTypes(queueFamily.queueFlags, queue.SupportedQueueTypes);
			for (const auto type : details.RequiredQueueTypes) {
				if (queueFamily.queueFlags & QueueTypeToVulkanType(type)) {
					requiredQueueTypesExist = true;
					// Graphics commands should get priority
					if(type == QueueInfo::QueueType::QUEUE_GRAPHICS_BIT) queue.Priority = 1.0f;
				}
			}
			int queueIndex = 0;
			while (queueIndex < queueFamily.queueCount) {
				m_QueueFamiliesPriorities[i].push_back(queue.Priority);
				queueIndex++;
				queue.QueueIndex = queueIndex;
				m_Queues.push_back(queue);
			}

			for (const auto feature : details.RequiredFeatures) { 
				VkBool32 presentSupport;
				if (feature == Feature::PRESENTATION) {
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, details.Surface, &presentSupport);
					if (presentSupport) {
						queue.PresentSupport = true;
					}
				}
			}
			i++;
		}
		if (!requiredQueueTypesExist) {
			return false;
			std::cout << "Required Queue Type does not exist";
		}
		return true;
	}

	bool GPUDeviceVulkan::IsDeviceSuitable(VkPhysicalDevice device, const Details& details) {
		if (!CreateQueueFamilies(device, details))
			return false;

		std::vector<const char*>& extensions = GPUContextVulkan::s_RequiredExtensions;
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
		
		//bool suitable = m_Indices.IsComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;

		// TODO: Check if required features are available too
		return true;
	}
}