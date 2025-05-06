// INCLUDES
#include "GpuAdapterVulkan.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/UtilitiesVulkan.hpp"
//--------------------------------------------
#include <set>
#include <iostream>
#include "TextureVulkan.hpp"
#include "DescriptorLayoutVulkan.hpp"
//-----------------

namespace Brisk
{
	std::vector<VkPhysicalDevice> GpuAdapterVulkan::RetrieveAvailableDevice(VkInstance instance) {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		std::vector<VkPhysicalDevice> devices;
		if (device_count == 0) {
			BRISK_CORE_ERROR("Failed to find GPUs with Vulkan support!");
			return devices;
		}
		devices.resize(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		return devices;
	}

	void GpuAdapterVulkan::Init() {
		volkInitialize();

		m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Engine::s_EngineSettings.EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		m_Extensions = UtilitiesVulkan::GetRequiredExtensions();
		m_ValidationLayersFound = false;
#if _DEBUG
		m_ValidationLayersFound = UtilitiesVulkan::CheckValidationLayerSupport(m_ValidationLayers);
		if (!m_ValidationLayersFound) {
			BRISK_APP_ERROR("Validation layers not found");
		}
#endif
		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_Extensions.size());
		createInfo.ppEnabledExtensionNames = m_Extensions.data();
#if _DEBUG
		createInfo.enabledLayerCount =
			m_ValidationLayersFound ? static_cast<uint32_t>(m_ValidationLayers.size()) : 0;
		createInfo.ppEnabledLayerNames =
			m_ValidationLayersFound ? m_ValidationLayers.data() : nullptr;

		UtilitiesVulkan::PopulateDebugMessengerCreateInfo(s_DebugCreateInfo);
		createInfo.pNext = &s_DebugCreateInfo;
#endif
		if ((vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)) {
			std::cout << "Failed to create Vulkan instance";
		}

		volkLoadInstance(m_Instance);

#if _DEBUG
		VkResult result = UtilitiesVulkan::CreateDebugUtilsMessengerEXT(m_Instance, s_DebugCreateInfo, s_DebugMessenger);
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			BRISK_APP_ERROR("Debug Utils extension not present");
		}
		else if (result != VK_SUCCESS) {
			BRISK_APP_ERROR("Failed to create debug messenger");
		}
#endif

		m_RequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

		std::vector<QueueType> queueTypes;
		std::vector<DeviceFeatures> features;
		GpuAdapterVulkan::GpuRequirements req{};

		std::vector<VkPhysicalDevice> availableDevices = RetrieveAvailableDevice(m_Instance);
		bool deviceFound = false;
		for (const auto& device : availableDevices) {
			if (IsDeviceSuitable(device, req)) {
				SetPhysicalDevice(device);
				deviceFound = true;
				break;
			}
		}
		if (!deviceFound) {
			BRISK_CORE_ERROR("Failed to find a suitable GPU!");
		}

		m_Surface = SurfaceFactoryVulkan::CreateNativeSurface(m_Instance);

		CreateLogicalDevice(req);

		AllocatePools();
	}

	void GpuAdapterVulkan::AllocatePools() {

		std::vector<VkDescriptorPoolSize> poolSizes{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 1024 },
			{ VK_DESCRIPTOR_TYPE_SAMPLER,				 1024 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		 1024 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 1024 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 1024 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 1024 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 32;

		if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		{
			uint32_t maxBindessResources = 1024;
			std::vector< VkDescriptorPoolSize> poolSizesBindless =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxBindessResources },
				//{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 maxBindessResources },
			};

			// Update after bind is needed here, for each binding and in the descriptor set layout creation.
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
			poolInfo.maxSets = 1;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizesBindless.size());
			poolInfo.pPoolSizes = poolSizesBindless.data();
			if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_BindlessDescriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create bindless descriptor pool!");
			}

			std::vector<VkDescriptorSetLayoutBinding> bindings;

			VkDescriptorSetLayoutBinding imageSamplerBinding{};
			imageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageSamplerBinding.descriptorCount = maxBindessResources;
			imageSamplerBinding.binding = SET_BINDLESS;
			imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
			imageSamplerBinding.pImmutableSamplers = nullptr;
			bindings.push_back(imageSamplerBinding);

			//VkDescriptorSetLayoutBinding storage_image_binding{};
			//storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			//storage_image_binding.descriptorCount = maxBindessResources;
			//storage_image_binding.binding = bindlessBinding + 1;
			//storage_image_binding.stageFlags = VK_SHADER_STAGE_ALL;
			//storage_image_binding.pImmutableSamplers = nullptr;
			//bindings.push_back(storage_image_binding);

			VkDescriptorSetLayoutCreateInfo layoutinfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutinfo.bindingCount = static_cast<uint32_t>(poolSizesBindless.size());
			layoutinfo.pBindings = bindings.data();
			layoutinfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

			VkDescriptorBindingFlags bindlessflags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | /*VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |*/ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
			std::vector< VkDescriptorBindingFlags> bindingflagsList{
				bindlessflags,
				//bindlessflags,
			};
			VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
			extended_info.bindingCount = static_cast<uint32_t>(poolSizesBindless.size());
			extended_info.pBindingFlags = bindingflagsList.data();

			layoutinfo.pNext = &extended_info;

			if (vkCreateDescriptorSetLayout(m_Device, &layoutinfo, nullptr, &m_BindlessDescriptorLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create bindless descriptor pool!");
			}

			VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfo.descriptorPool = m_BindlessDescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_BindlessDescriptorLayout;

			VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
			uint32_t maxBinding = maxBindessResources - 1;
			countInfo.descriptorSetCount = 1;
			// This number is the max allocatable count
			countInfo.pDescriptorCounts = &maxBinding;
			allocInfo.pNext = &countInfo;

			if (vkAllocateDescriptorSets(m_Device, &allocInfo, &m_BindlessTexturesSet) != VK_SUCCESS) {
				throw std::runtime_error("failed to create bindless descriptor pool!");
			}
		}
	}

	void GpuAdapterVulkan::CreateLogicalDevice(const GpuRequirements& requirements) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		std::vector<QueueFamily> QueuFamilies{};
		// Requesting all available queues
		std::vector<std::vector<float>> queuePrioritiesList;
		queuePrioritiesList.resize(queueFamilyCount);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			bool isGraphics = false;
			bool isCompute = false;
			bool isTransfer = false;
			QueueFamily queueFamily{};
			queueFamily.Index = i;
			queueFamily.QueueCount = queueFamilies[i].queueCount;

			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface->GetSurface(), &presentSupport);
			queueFamily.PresentSupport = presentSupport;

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				isGraphics = true;
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_GRAPHICS_BIT);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				isCompute = true;
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_COMPUTE_BIT);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
				isTransfer = true;
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_TRANSFER_BIT);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_SPARSE_BINDING_BIT);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_PROTECTED_BIT);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_VIDEO_DECODE_BIT_KHR);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_VIDEO_ENCODE_BIT_KHR);
			}
			if (queueFamilies[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
				queueFamily.SupportedTypes.push_back(GpuAdapterVulkan::QueueType::QUEUE_OPTICAL_FLOW_BIT_NV);
			}

			if (isTransfer && !isGraphics && !isCompute)
				queueFamily.IsExplicitTransferQueue = true;
			if (isCompute && !isGraphics && !isTransfer)
				queueFamily.IsExplicitComputeQueue = true;

			VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueCreateInfo.queueFamilyIndex = i;
			queueCreateInfo.queueCount = queueFamilies[i].queueCount;

			queuePrioritiesList[i].resize(queueFamilies[i].queueCount, 1.0f);
			queueCreateInfo.pQueuePriorities = queuePrioritiesList[i].data();

			QueuFamilies.push_back(queueFamily);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = requirements.pFeatures.pSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		deviceFeatures.robustBufferAccess = requirements.pFeatures.pRobustBufferAccess ? VK_TRUE : VK_FALSE;
		deviceFeatures.fullDrawIndexUint32 = requirements.pFeatures.pFullDrawIndexUint32 ? VK_TRUE : VK_FALSE;
		deviceFeatures.imageCubeArray = requirements.pFeatures.pImageCubeArray ? VK_TRUE : VK_FALSE;
		deviceFeatures.independentBlend = requirements.pFeatures.pIndependentBlend ? VK_TRUE : VK_FALSE;
		deviceFeatures.geometryShader = requirements.pFeatures.pGeometryShader ? VK_TRUE : VK_FALSE;
		deviceFeatures.tessellationShader = requirements.pFeatures.pTessellationShader ? VK_TRUE : VK_FALSE;
		deviceFeatures.sampleRateShading = requirements.pFeatures.pSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		deviceFeatures.dualSrcBlend = requirements.pFeatures.pDualSrcBlend ? VK_TRUE : VK_FALSE;
		deviceFeatures.logicOp = requirements.pFeatures.pLogicOp ? VK_TRUE : VK_FALSE;
		deviceFeatures.multiDrawIndirect = requirements.pFeatures.pMultiDrawIndirect ? VK_TRUE : VK_FALSE;
		deviceFeatures.drawIndirectFirstInstance = requirements.pFeatures.pDrawIndirectFirstInstance ? VK_TRUE : VK_FALSE;
		deviceFeatures.depthClamp = requirements.pFeatures.pDepthClamp ? VK_TRUE : VK_FALSE;
		deviceFeatures.depthBiasClamp = requirements.pFeatures.pDepthBiasClamp ? VK_TRUE : VK_FALSE;
		deviceFeatures.fillModeNonSolid = requirements.pFeatures.pFillModeNonSolid ? VK_TRUE : VK_FALSE;
		deviceFeatures.depthBounds = requirements.pFeatures.pDepthBounds ? VK_TRUE : VK_FALSE;
		deviceFeatures.wideLines = requirements.pFeatures.pWideLines ? VK_TRUE : VK_FALSE;
		deviceFeatures.largePoints = requirements.pFeatures.pLargePoints ? VK_TRUE : VK_FALSE;
		deviceFeatures.alphaToOne = requirements.pFeatures.pAlphaToOne ? VK_TRUE : VK_FALSE;
		deviceFeatures.multiViewport = requirements.pFeatures.pMultiViewport ? VK_TRUE : VK_FALSE;
		deviceFeatures.samplerAnisotropy = requirements.pFeatures.pSamplerAnisotropy ? VK_TRUE : VK_FALSE;
		deviceFeatures.textureCompressionETC2 = requirements.pFeatures.pTextureCompressionETC2 ? VK_TRUE : VK_FALSE;
		deviceFeatures.textureCompressionASTC_LDR = requirements.pFeatures.pTextureCompressionASTC_LDR ? VK_TRUE : VK_FALSE;
		deviceFeatures.textureCompressionBC = requirements.pFeatures.pTextureCompressionBC ? VK_TRUE : VK_FALSE;
		deviceFeatures.occlusionQueryPrecise = requirements.pFeatures.pOcclusionQueryPrecise ? VK_TRUE : VK_FALSE;
		deviceFeatures.pipelineStatisticsQuery = requirements.pFeatures.pPipelineStatisticsQuery ? VK_TRUE : VK_FALSE;
		deviceFeatures.vertexPipelineStoresAndAtomics = requirements.pFeatures.pVertexPipelineStoresAndAtomics ? VK_TRUE : VK_FALSE;
		deviceFeatures.fragmentStoresAndAtomics = requirements.pFeatures.pFragmentStoresAndAtomics ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderTessellationAndGeometryPointSize = requirements.pFeatures.pShaderTessellationAndGeometryPointSize ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderImageGatherExtended = requirements.pFeatures.pShaderImageGatherExtended ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageImageExtendedFormats = requirements.pFeatures.pShaderStorageImageExtendedFormats ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageImageMultisample = requirements.pFeatures.pShaderStorageImageMultisample ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageImageReadWithoutFormat = requirements.pFeatures.pShaderStorageImageReadWithoutFormat ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageImageWriteWithoutFormat = requirements.pFeatures.pShaderStorageImageWriteWithoutFormat ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderUniformBufferArrayDynamicIndexing = requirements.pFeatures.pShaderUniformBufferArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderSampledImageArrayDynamicIndexing = requirements.pFeatures.pShaderSampledImageArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageBufferArrayDynamicIndexing = requirements.pFeatures.pShaderStorageBufferArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderStorageImageArrayDynamicIndexing = requirements.pFeatures.pShaderStorageImageArrayDynamicIndexing ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderClipDistance = requirements.pFeatures.pShaderClipDistance ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderCullDistance = requirements.pFeatures.pShaderCullDistance ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderFloat64 = requirements.pFeatures.pShaderFloat64 ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderInt64 = requirements.pFeatures.pShaderInt64 ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderInt16 = requirements.pFeatures.pShaderInt16 ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderResourceResidency = requirements.pFeatures.pShaderResourceResidency ? VK_TRUE : VK_FALSE;
		deviceFeatures.shaderResourceMinLod = requirements.pFeatures.pShaderResourceMinLod ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseBinding = requirements.pFeatures.pSparseBinding ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidencyBuffer = requirements.pFeatures.pSparseResidencyBuffer ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidencyImage2D = requirements.pFeatures.pSparseResidencyImage2D ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidencyImage3D = requirements.pFeatures.pSparseResidencyImage3D ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidency2Samples = requirements.pFeatures.pSparseResidency2Samples ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidency4Samples = requirements.pFeatures.pSparseResidency4Samples ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidency8Samples = requirements.pFeatures.pSparseResidency8Samples ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidency16Samples = requirements.pFeatures.pSparseResidency16Samples ? VK_TRUE : VK_FALSE;
		deviceFeatures.sparseResidencyAliased = requirements.pFeatures.pSparseResidencyAliased ? VK_TRUE : VK_FALSE;
		deviceFeatures.variableMultisampleRate = requirements.pFeatures.pVariableMultisampleRate ? VK_TRUE : VK_FALSE;
		deviceFeatures.inheritedQueries = requirements.pFeatures.pInheritedQueries ? VK_TRUE : VK_FALSE;

		VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint16_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		//deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		//std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		deviceCreateInfo.enabledExtensionCount = static_cast<uint16_t>(m_RequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();

		VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
		VkPhysicalDeviceFeatures2 physical_features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,&indexing_features };
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &physical_features2);

		deviceCreateInfo.pNext = &physical_features2;
		physical_features2.pNext = &indexing_features;

#if _DEBUG
		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		deviceCreateInfo.ppEnabledLayerNames = validation_layers.data();
#else
		device_create_info.enabledLayerCount = 0;
#endif
		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create logical device!");
		}

		queuePrioritiesList.clear();
		for (QueueFamily queue : QueuFamilies) {
			for (QueueType type : queue.SupportedTypes) {
				uint32_t i = 0;
				if (type == QUEUE_GRAPHICS_BIT) {
					vkGetDeviceQueue(m_Device, queue.Index, i++, &m_GraphicsQueue.Handle);
					if (i >= queue.QueueCount) break;
				}
				if (queue.IsExplicitTransferQueue) {
					vkGetDeviceQueue(m_Device, queue.Index, i++, &m_TransferQueue.Handle);
					if (i >= queue.QueueCount) break;
				}
				if (queue.IsExplicitComputeQueue) {
					vkGetDeviceQueue(m_Device, queue.Index, i++, &m_ComputeQueue.Handle);
					if (i >= queue.QueueCount) break;
				}
			}
		}
	}

	void GpuAdapterVulkan::Release() {
		vkDestroyDevice(m_Device, nullptr);
	}

	bool GpuAdapterVulkan::IsDeviceSuitable(VkPhysicalDevice device, const GpuRequirements& requirements) {
		std::vector<const char*>& extensions = m_RequiredExtensions;
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

		VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
		VkPhysicalDeviceFeatures2 device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,&indexing_features };
		vkGetPhysicalDeviceFeatures2(device, &device_features);

		bool bindless_supported = indexing_features.descriptorBindingPartiallyBound && indexing_features.runtimeDescriptorArray;
		assert(bindless_supported); // Solely dependent on bindless rendering for now

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

	void GpuAdapterVulkan::SetupDescriptorSets(std::vector<std::shared_ptr<DescriptorLayout>> descriptorLayouts) {
		for (const auto& l : descriptorLayouts) {
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			VkDescriptorSetLayout layout = std::static_pointer_cast<DescriptorLayoutVulkan>(l)->GetLayout();

			allocInfo.descriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDescriptorPool();
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;
			switch (l->GetDescriptorType())
			{
			case Brisk::MVPUBO:
			{
				if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_MVPUBOSet) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				}
				break;
			}
			case Brisk::BindlessTextures:
			{
				// Already allocated
				break;
			}
			case Brisk::SceneLightsUBO:
			{
				if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_SceneLightsSet) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				}
				break;
			}
			case Brisk::SceneTextures:
			{
				if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_TexturesSet) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				}
				break;
			}
			default:
				BRISK_CORE_ERROR("Not implemented yet");
				break;
			}
		}
	}

	void GpuAdapterVulkan::AddResource(GpuDescriptorResourceType type, std::shared_ptr<Texture> texture, std::shared_ptr<Buffer> buffer, int bindingIndex) {
		switch (type)
		{
		case Brisk::MVPUBO:
		{
			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstSet = m_MVPUBOSet;
			write.dstBinding = bindingIndex;
			write.descriptorCount = 1;
			write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();
			vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
			break;
		}
		case Brisk::SceneLightsUBO:
		{
			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstSet = m_SceneLightsSet;
			write.dstBinding = bindingIndex;
			write.descriptorCount = 1;
			write.pBufferInfo = std::static_pointer_cast<BufferVulkan>(buffer)->GetDescriptor();
			vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
			break;
		}
		case Brisk::SceneTextures:
		{
			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessTexturesSet;
			write.dstBinding = 0;
			write.dstArrayElement = bindingIndex;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();

			vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
			break;
		}
		case Brisk::BindlessTextures:
		{
			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->m_BindlessTexturesSet;
			write.dstBinding = 0;
			write.dstArrayElement = bindingIndex;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;
			write.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();

			vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
			break;
		}
		default:
			break;
		}
	}
}