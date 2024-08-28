#include "GpuDeviceVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"

#include <set>

namespace Brisk 
{
	std::vector<VkPhysicalDevice> GpuDeviceVulkan::RetrieveAvailableDevice() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(GpuContextVulkan::s_Instance, &device_count, nullptr);
		if (device_count == 0) {
			BRISK_CORE_ERROR("Failed to find GPUs with Vulkan support!");
			return;
		}
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(GpuContextVulkan::s_Instance, &device_count, devices.data());

		return devices;
	}

	void GpuDeviceVulkan::CreateLogicalDevice(const GpuRequirements& requirements) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		// Requesting all availabel queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = i;
			queueCreateInfo.queueCount = queueFamilies[i].queueCount;

			std::vector<float> queuePriorities(queueFamilies[i].queueCount, 1.0f);
			queueCreateInfo.pQueuePriorities = queuePriorities.data();

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = requirements.pFeatures.pSamplerAnisotropy? VK_TRUE : VK_FALSE;
		device_features.robustBufferAccess = requirements.pFeatures.pRobustBufferAccess ? VK_TRUE : VK_FALSE;
		device_features.fullDrawIndexUint32 = requirements.pFeatures.pFullDrawIndexUint32? VK_TRUE : VK_FALSE;
		device_features.imageCubeArray = requirements.pFeatures.pImageCubeArray? VK_TRUE : VK_FALSE;
		device_features.independentBlend = requirements.pFeatures.pIndependentBlend? VK_TRUE : VK_FALSE;
		device_features.geometryShader = requirements.pFeatures.pGeometryShader ? VK_TRUE : VK_FALSE;
		device_features.tessellationShader = requirements.pFeatures.pTessellationShader ? VK_TRUE : VK_FALSE;
		device_features.sampleRateShading = requirements.pFeatures.pSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		device_features.dualSrcBlend = requirements.pFeatures.pDualSrcBlend ? VK_TRUE : VK_FALSE;
		device_features.logicOp = requirements.pFeatures.pLogicOp ? VK_TRUE : VK_FALSE;
		device_features.multiDrawIndirect = requirements.pFeatures.pMultiDrawIndirect? VK_TRUE : VK_FALSE;
		device_features.drawIndirectFirstInstance = requirements.pFeatures.pDrawIndirectFirstInstance? VK_TRUE : VK_FALSE;
		device_features.depthClamp = requirements.pFeatures.pDepthClamp ? VK_TRUE : VK_FALSE;
		device_features.depthBiasClamp = requirements.pFeatures.pDepthBiasClamp ? VK_TRUE : VK_FALSE;
		device_features.fillModeNonSolid = requirements.pFeatures.pFillModeNonSolid ? VK_TRUE : VK_FALSE;
		device_features.depthBounds = requirements.pFeatures.pDepthBounds ? VK_TRUE : VK_FALSE;
		device_features.wideLines = requirements.pFeatures.pWideLines ? VK_TRUE : VK_FALSE;
		device_features.largePoints = requirements.pFeatures.pLargePoints ? VK_TRUE : VK_FALSE;
		device_features.alphaToOne = requirements.pFeatures.pAlphaToOne ? VK_TRUE : VK_FALSE;
		device_features.multiViewport = requirements.pFeatures.pMultiViewport ? VK_TRUE : VK_FALSE;
		device_features.samplerAnisotropy = requirements.pFeatures.pSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		device_features.textureCompressionETC2 = requirements.pFeatures.pTextureCompressionETC2 ? VK_TRUE : VK_FALSE;
		device_features.textureCompressionASTC_LDR = requirements.pFeatures.pTextureCompressionASTC_LDR? VK_TRUE : VK_FALSE;
		device_features.textureCompressionBC = requirements.pFeatures.pTextureCompressionBC ? VK_TRUE : VK_FALSE;
		device_features.occlusionQueryPrecise = requirements.pFeatures.pOcclusionQueryPrecise ? VK_TRUE : VK_FALSE;
		device_features.pipelineStatisticsQuery = requirements.pFeatures.pPipelineStatisticsQuery ? VK_TRUE : VK_FALSE;
		device_features.vertexPipelineStoresAndAtomics = requirements.pFeatures.pVertexPipelineStoresAndAtomics ? VK_TRUE : VK_FALSE;
		device_features.fragmentStoresAndAtomics = requirements.pFeatures.pFragmentStoresAndAtomics ? VK_TRUE : VK_FALSE;
		device_features.shaderTessellationAndGeometryPointSize = requirements.pFeatures.pShaderTessellationAndGeometryPointSize? VK_TRUE : VK_FALSE;
		device_features.shaderImageGatherExtended = requirements.pFeatures.pShaderImageGatherExtended ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageImageExtendedFormats = requirements.pFeatures.pShaderStorageImageExtendedFormats ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageImageMultisample = requirements.pFeatures.pShaderStorageImageMultisample ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageImageReadWithoutFormat = requirements.pFeatures.pShaderStorageImageReadWithoutFormat ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageImageWriteWithoutFormat = requirements.pFeatures.pShaderStorageImageWriteWithoutFormat ? VK_TRUE : VK_FALSE;
		device_features.shaderUniformBufferArrayDynamicIndexing = requirements.pFeatures.pShaderUniformBufferArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		device_features.shaderSampledImageArrayDynamicIndexing = requirements.pFeatures.pShaderSampledImageArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageBufferArrayDynamicIndexing = requirements.pFeatures.pShaderStorageBufferArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		device_features.shaderStorageImageArrayDynamicIndexing = requirements.pFeatures.pShaderStorageImageArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		device_features.shaderClipDistance = requirements.pFeatures.pShaderClipDistance ? VK_TRUE : VK_FALSE;
		device_features.shaderCullDistance = requirements.pFeatures.pShaderCullDistance ? VK_TRUE : VK_FALSE;
		device_features.shaderFloat64 = requirements.pFeatures.pShaderFloat64 ? VK_TRUE : VK_FALSE;
		device_features.shaderInt64 = requirements.pFeatures.pShaderInt64 ? VK_TRUE : VK_FALSE;
		device_features.shaderInt16 = requirements.pFeatures.pShaderInt16 ? VK_TRUE : VK_FALSE;
		device_features.shaderResourceResidency = requirements.pFeatures.pShaderResourceResidency ? VK_TRUE : VK_FALSE;
		device_features.shaderResourceMinLod = requirements.pFeatures.pShaderResourceMinLod ? VK_TRUE : VK_FALSE;
		device_features.sparseBinding = requirements.pFeatures.pSparseBinding ? VK_TRUE : VK_FALSE;
		device_features.sparseResidencyBuffer = requirements.pFeatures.pSparseResidencyBuffer ? VK_TRUE : VK_FALSE;
		device_features.sparseResidencyImage2D = requirements.pFeatures.pSparseResidencyImage2D ? VK_TRUE : VK_FALSE;
		device_features.sparseResidencyImage3D = requirements.pFeatures.pSparseResidencyImage3D ? VK_TRUE : VK_FALSE;
		device_features.sparseResidency2Samples = requirements.pFeatures.pSparseResidency2Samples ? VK_TRUE : VK_FALSE;
		device_features.sparseResidency4Samples = requirements.pFeatures.pSparseResidency4Samples ? VK_TRUE : VK_FALSE;
		device_features.sparseResidency8Samples = requirements.pFeatures.pSparseResidency8Samples ? VK_TRUE : VK_FALSE;
		device_features.sparseResidency16Samples = requirements.pFeatures.pSparseResidency16Samples ? VK_TRUE : VK_FALSE;
		device_features.sparseResidencyAliased = requirements.pFeatures.pSparseResidencyAliased ? VK_TRUE : VK_FALSE;
		device_features.variableMultisampleRate = requirements.pFeatures.pVariableMultisampleRate ? VK_TRUE : VK_FALSE;
		device_features.inheritedQueries = requirements.pFeatures.pInheritedQueries ? VK_TRUE : VK_FALSE;

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
	}

	void GpuDeviceVulkan::Release() {
		delete m_PresentQueue;
		delete m_GraphicsQueue;
		vkDestroyDevice(m_Device, nullptr);
	}

	//const std::vector<GpuDeviceVulkan::QueueInfo> GpuDeviceVulkan::RetrieveCommonQueues() {
	//	std::vector<GpuDeviceVulkan::QueueInfo> queues;
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

	//const std::string& GpuDeviceVulkan::QueueTypeToString(QueueInfo::QueueType type) {
	//	switch (type)
	//	{
	//	case QueueInfo::QueueType::QUEUE_GRAPHICS_BIT:
	//		return "VK_QUEUE_GRAPHICS_BIT";
	//	case QueueInfo::QueueType::QUEUE_COMPUTE_BIT:
	//		return "VK_QUEUE_COMPUTE_BIT";
	//	case QueueInfo::QueueType::QUEUE_TRANSFER_BIT:
	//		return "VK_QUEUE_TRANSFER_BIT";
	//	case QueueInfo::QueueType::QUEUE_SPARSE_BINDING_BIT:
	//		return "VK_QUEUE_SPARSE_BINDING_BIT";
	//	case QueueInfo::QueueType::QUEUE_PROTECTED_BIT:
	//		return "VK_QUEUE_PROTECTED_BIT";
	//	case QueueInfo::QueueType::QUEUE_VIDEO_DECODE_BIT_KHR:
	//		return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
	//	case QueueInfo::QueueType::QUEUE_VIDEO_ENCODE_BIT_KHR:
	//		return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
	//	case QueueInfo::QueueType::QUEUE_OPTICAL_FLOW_BIT_NV:
	//		return "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
	//	default:
	//		// Error
	//		return "VK_QUEUE_FLAG_BITS_MAX_ENUM";
	//	}
	//}

	//void GpuDeviceVulkan::PrintQueueFlags(VkQueueFlags flags) {
	//	if (flags & VK_QUEUE_GRAPHICS_BIT)
	//		std::cout << "VK_QUEUE_GRAPHICS_BIT" << std::endl;
	//	if (flags & VK_QUEUE_COMPUTE_BIT)
	//		std::cout << "VK_QUEUE_COMPUTE_BIT" << std::endl;
	//	if (flags & VK_QUEUE_TRANSFER_BIT)
	//		std::cout << "VK_QUEUE_TRANSFER_BIT" << std::endl;
	//	if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
	//		std::cout << "VK_QUEUE_SPARSE_BINDING_BIT" << std::endl;
	//	if (flags & VK_QUEUE_PROTECTED_BIT)
	//		std::cout << "VK_QUEUE_PROTECTED_BIT" << std::endl;
	//	if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
	//		std::cout << "VK_QUEUE_VIDEO_DECODE_BIT_KHR" << std::endl;
	//	if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
	//		std::cout << "VK_QUEUE_VIDEO_ENCODE_BIT_KHR" << std::endl;
	//	if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
	//		std::cout << "VK_QUEUE_OPTICAL_FLOW_BIT_NV" << std::endl;
	//}

	//void GpuDeviceVulkan::GetSupportedQueueTypes(VkQueueFlags flags, std::vector<QueueInfo::QueueType>& ref) {
	//	if (flags & VK_QUEUE_GRAPHICS_BIT)
	//		ref.push_back(QueueInfo::QUEUE_GRAPHICS_BIT);
	//	if (flags & VK_QUEUE_COMPUTE_BIT)
	//		ref.push_back(QueueInfo::QUEUE_COMPUTE_BIT);
	//	if (flags & VK_QUEUE_TRANSFER_BIT)
	//		ref.push_back(QueueInfo::QUEUE_TRANSFER_BIT);
	//	if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
	//		ref.push_back(QueueInfo::QUEUE_SPARSE_BINDING_BIT);
	//	if (flags & VK_QUEUE_PROTECTED_BIT)
	//		ref.push_back(QueueInfo::QUEUE_PROTECTED_BIT);
	//	if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
	//		ref.push_back(QueueInfo::QUEUE_VIDEO_DECODE_BIT_KHR);
	//	if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
	//		ref.push_back(QueueInfo::QUEUE_VIDEO_ENCODE_BIT_KHR);
	//	if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
	//		ref.push_back(QueueInfo::QUEUE_OPTICAL_FLOW_BIT_NV);
	//}

	//VkQueueFlags GpuDeviceVulkan::QueueTypeToVulkanType(QueueInfo::QueueType type) {
	//	switch (type)
	//	{
	//		case QueueInfo::QueueType::QUEUE_GRAPHICS_BIT:
	//			return VK_QUEUE_GRAPHICS_BIT;
	//		case QueueInfo::QueueType::QUEUE_COMPUTE_BIT:
	//			return VK_QUEUE_COMPUTE_BIT;
	//		case QueueInfo::QueueType::QUEUE_TRANSFER_BIT:
	//			return VK_QUEUE_TRANSFER_BIT;
	//		case QueueInfo::QueueType::QUEUE_SPARSE_BINDING_BIT:
	//			return VK_QUEUE_SPARSE_BINDING_BIT;
	//		case QueueInfo::QueueType::QUEUE_PROTECTED_BIT:
	//			return VK_QUEUE_PROTECTED_BIT;
	//		case QueueInfo::QueueType::QUEUE_VIDEO_DECODE_BIT_KHR:
	//			return VK_QUEUE_VIDEO_DECODE_BIT_KHR;
	//		case QueueInfo::QueueType::QUEUE_VIDEO_ENCODE_BIT_KHR:
	//			return VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
	//		case QueueInfo::QueueType::QUEUE_OPTICAL_FLOW_BIT_NV:
	//			return VK_QUEUE_OPTICAL_FLOW_BIT_NV;
	//		default:
	//			// Error
	//			return VK_QUEUE_FLAG_BITS_MAX_ENUM;
	//	}
	//}

	bool GpuDeviceVulkan::IsDeviceSuitable(VkPhysicalDevice device, const GpuRequirements& requirements) {
		std::vector<const char*>& extensions = GpuContextVulkan::s_RequiredExtensions;
		bool extensionsSupported = false;
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		extensionsSupported = requiredExtensions.empty();

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		if (requirements.pFeatures.pRobustBufferAccess) if (!supportedFeatures.robustBufferAccess) return false;
		if (requirements.pFeatures.pFullDrawIndexUint32) if (!supportedFeatures.fullDrawIndexUint32) return false;
		if (requirements.pFeatures.pImageCubeArray) if (!supportedFeatures.imageCubeArray) return false;
		if (requirements.pFeatures.pIndependentBlend) if (!supportedFeatures.independentBlend) return false;
		if (requirements.pFeatures.pGeometryShader) if (!supportedFeatures.geometryShader) return false;
		if (requirements.pFeatures.pTessellationShader) if (!supportedFeatures.tessellationShader) return false;
		if (requirements.pFeatures.pSampleRateShading) if (!supportedFeatures.sampleRateShading) return false;
		if (requirements.pFeatures.pDualSrcBlend) if (!supportedFeatures.dualSrcBlend) return false;
		if (requirements.pFeatures.pLogicOp) if (!supportedFeatures.logicOp) return false;
		if (requirements.pFeatures.pMultiDrawIndirect) if (!supportedFeatures.multiDrawIndirect) return false;
		if (requirements.pFeatures.pDrawIndirectFirstInstance) if (!supportedFeatures.drawIndirectFirstInstance) return false;
		if (requirements.pFeatures.pDepthClamp) if (!supportedFeatures.depthClamp) return false;
		if (requirements.pFeatures.pDepthBiasClamp) if (!supportedFeatures.depthBiasClamp) return false;
		if (requirements.pFeatures.pFillModeNonSolid) if (!supportedFeatures.fillModeNonSolid) return false;
		if (requirements.pFeatures.pDepthBounds) if (!supportedFeatures.depthBounds) return false;
		if (requirements.pFeatures.pWideLines) if (!supportedFeatures.wideLines) return false;
		if (requirements.pFeatures.pLargePoints) if (!supportedFeatures.largePoints) return false;
		if (requirements.pFeatures.pAlphaToOne) if (!supportedFeatures.alphaToOne) return false;
		if (requirements.pFeatures.pMultiViewport) if (!supportedFeatures.multiViewport) return false;
		if (requirements.pFeatures.pSamplerAnisotropy) if (!supportedFeatures.samplerAnisotropy) return false;
		if (requirements.pFeatures.pTextureCompressionETC2) if (!supportedFeatures.textureCompressionETC2) return false;
		if (requirements.pFeatures.pTextureCompressionASTC_LDR) if (!supportedFeatures.textureCompressionASTC_LDR) return false;
		if (requirements.pFeatures.pTextureCompressionBC) if (!supportedFeatures.textureCompressionBC) return false;
		if (requirements.pFeatures.pOcclusionQueryPrecise) if (!supportedFeatures.occlusionQueryPrecise) return false;
		if (requirements.pFeatures.pPipelineStatisticsQuery) if (!supportedFeatures.pipelineStatisticsQuery) return false;
		if (requirements.pFeatures.pVertexPipelineStoresAndAtomics) if (!supportedFeatures.vertexPipelineStoresAndAtomics) return false;
		if (requirements.pFeatures.pFragmentStoresAndAtomics) if (!supportedFeatures.fragmentStoresAndAtomics) return false;
		if (requirements.pFeatures.pShaderTessellationAndGeometryPointSize) if (!supportedFeatures.shaderTessellationAndGeometryPointSize) return false;
		if (requirements.pFeatures.pShaderImageGatherExtended) if (!supportedFeatures.shaderImageGatherExtended) return false;
		if (requirements.pFeatures.pShaderStorageImageExtendedFormats) if (!supportedFeatures.shaderStorageImageExtendedFormats) return false;
		if (requirements.pFeatures.pShaderStorageImageMultisample) if (!supportedFeatures.shaderStorageImageMultisample) return false;
		if (requirements.pFeatures.pShaderStorageImageReadWithoutFormat) if (!supportedFeatures.shaderStorageImageReadWithoutFormat) return false;
		if (requirements.pFeatures.pShaderStorageImageWriteWithoutFormat) if (!supportedFeatures.shaderStorageImageWriteWithoutFormat) return false;
		if (requirements.pFeatures.pShaderUniformBufferArrayDynamicIndexing) if (!supportedFeatures.shaderUniformBufferArrayDynamicIndexing) return false;
		if (requirements.pFeatures.pShaderSampledImageArrayDynamicIndexing) if (!supportedFeatures.shaderSampledImageArrayDynamicIndexing) return false;
		if (requirements.pFeatures.pShaderStorageBufferArrayDynamicIndexing) if (!supportedFeatures.shaderStorageBufferArrayDynamicIndexing) return false;
		if (requirements.pFeatures.pShaderStorageImageArrayDynamicIndexing) if (!supportedFeatures.shaderStorageImageArrayDynamicIndexing) return false;
		if (requirements.pFeatures.pShaderClipDistance) if (!supportedFeatures.shaderClipDistance) return false;
		if (requirements.pFeatures.pShaderCullDistance) if (!supportedFeatures.shaderCullDistance) return false;
		if (requirements.pFeatures.pShaderFloat64) if (!supportedFeatures.shaderFloat64) return false;
		if (requirements.pFeatures.pShaderInt64) if (!supportedFeatures.shaderInt64) return false;
		if (requirements.pFeatures.pShaderInt16) if (!supportedFeatures.shaderInt16) return false;
		if (requirements.pFeatures.pShaderResourceResidency) if (!supportedFeatures.shaderResourceResidency) return false;
		if (requirements.pFeatures.pShaderResourceMinLod) if (!supportedFeatures.shaderResourceMinLod) return false;
		if (requirements.pFeatures.pSparseBinding) if (!supportedFeatures.sparseBinding) return false;
		if (requirements.pFeatures.pSparseResidencyBuffer) if (!supportedFeatures.sparseResidencyBuffer) return false;
		if (requirements.pFeatures.pSparseResidencyImage2D) if (!supportedFeatures.sparseResidencyImage2D) return false;
		if (requirements.pFeatures.pSparseResidencyImage3D) if (!supportedFeatures.sparseResidencyImage3D) return false;
		if (requirements.pFeatures.pSparseResidency2Samples) if (!supportedFeatures.sparseResidency2Samples) return false;
		if (requirements.pFeatures.pSparseResidency4Samples) if (!supportedFeatures.sparseResidency4Samples) return false;
		if (requirements.pFeatures.pSparseResidency8Samples) if (!supportedFeatures.sparseResidency8Samples) return false;
		if (requirements.pFeatures.pSparseResidency16Samples) if (!supportedFeatures.sparseResidency16Samples) return false;
		if (requirements.pFeatures.pSparseResidencyAliased) if (!supportedFeatures.sparseResidencyAliased) return false;
		if (requirements.pFeatures.pVariableMultisampleRate) if (!supportedFeatures.variableMultisampleRate) return false;
		if (requirements.pFeatures.pInheritedQueries) if (!supportedFeatures.inheritedQueries) return false;

		if (!extensionsSupported) return false;

		// Device is suitable!
		return true;
	}
}