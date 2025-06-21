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

#include <volk.h>
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
		appInfo.pApplicationName = "Potato";
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

		m_RequiredExtensions = 
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		std::vector<VkPhysicalDevice> availableDevices = RetrieveAvailableDevice(m_Instance);
		bool deviceFound = false;
		for (const auto& device : availableDevices) {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				SetPhysicalDevice(device);
				deviceFound = true;
				break;
			}
		}

		if (!deviceFound && !availableDevices.empty()) {
			SetPhysicalDevice(availableDevices[0]);
			deviceFound = true;
		}

		if (!deviceFound) {
			BRISK_CORE_ERROR("Failed to find a suitable GPU!");
		}

		m_Surface = SurfaceFactoryVulkan::CreateNativeSurface(m_Instance);

		CreateLogicalDevice();
		volkLoadDevice(m_Device);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = m_PhysicalDevice;
		allocatorInfo.device = m_Device;
		allocatorInfo.instance = m_Instance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
		VmaVulkanFunctions vulkanFunctions = {};
		VkResult res = vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vulkanFunctions);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to import Vulkan functions from Volk");
		}

		// Pass the filled struct pointer to allocatorInfo
		allocatorInfo.pVulkanFunctions = &vulkanFunctions;

		res = vmaCreateAllocator(&allocatorInfo, &m_VmaAllocator);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create VMA allocator.");
		}

		AllocatePools();

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->Init();
			m_DummyDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->SetDescriptorType(GpuDescriptorResourceType::MVPUBO);
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT });
			layout->Init();
			m_MVPDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->SetDescriptorType(GpuDescriptorResourceType::SceneLightsUBO);
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->Init();
			m_LightsDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->SetDescriptorType(GpuDescriptorResourceType::DeferredTextures);
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->Init();
			m_DeferredTexturesDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_MVPDescriptorLayout;
		if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_MVPUBOSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
		allocInfo.pSetLayouts = &m_LightsDescriptorLayout;
		if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_SceneLightsSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		allocInfo.pSetLayouts = &m_DeferredTexturesDescriptorLayout;
		if (vkAllocateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_DeferredTexturesSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = m_GraphicsQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
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

		// Setup bindless rendering
		{
			uint32_t maxBindlessResources = 1024;
			std::vector< VkDescriptorPoolSize> poolSizesBindless =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxBindlessResources },
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
			imageSamplerBinding.descriptorCount = maxBindlessResources;
			imageSamplerBinding.binding = 0;
			imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
			imageSamplerBinding.pImmutableSamplers = nullptr;
			bindings.push_back(imageSamplerBinding);

			//VkDescriptorSetLayoutBinding storage_image_binding{};
			//storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			//storage_image_binding.descriptorCount = maxBindlessResources;
			//storage_image_binding.binding = bindlessBinding + 1;
			//storage_image_binding.stageFlags = VK_SHADER_STAGE_ALL;
			//storage_image_binding.pImmutableSamplers = nullptr;
			//bindings.push_back(storage_image_binding);

			VkDescriptorSetLayoutCreateInfo layoutinfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutinfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutinfo.pBindings = bindings.data();
			layoutinfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

			VkDescriptorBindingFlags bindlessflags = 
				VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | 
				VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | 
				VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

			std::vector<VkDescriptorBindingFlags> bindingflagsList(bindings.size(), bindlessflags);
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
			uint32_t maxBinding = maxBindlessResources;
			countInfo.descriptorSetCount = 1;
			// This number is the max allocatable count
			countInfo.pDescriptorCounts = &maxBinding;
			allocInfo.pNext = &countInfo;

			if (vkAllocateDescriptorSets(m_Device, &allocInfo, &m_BindlessTexturesSet) != VK_SUCCESS) {
				throw std::runtime_error("failed to create bindless descriptor pool!");
			}
		}
	}

	void GpuAdapterVulkan::CreateLogicalDevice() {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		// Creating queue create infos
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> skipFamilies;
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			float queuePriority = 1.0f;
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface->GetSurface(), &presentSupport);
			if (presentSupport && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				skipFamilies.insert(i);
				VkDeviceQueueCreateInfo graphicsQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				m_GraphicsQueueFamily = i;
				graphicsQueueCreateInfo.queueFamilyIndex = i;
				graphicsQueueCreateInfo.queueCount = 1;
				graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(graphicsQueueCreateInfo);
				break;
			}
		}
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (std::find(skipFamilies.begin(), skipFamilies.end(), i) != skipFamilies.end())
				continue;
			float queuePriority = 1.0f;
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
				VkDeviceQueueCreateInfo transferQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				m_TransferQueueFamily = i;
				transferQueueCreateInfo.queueFamilyIndex = i;
				transferQueueCreateInfo.queueCount = 1;
				transferQueueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(transferQueueCreateInfo);
				break;
			}
		}

		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
			BRISK_CORE_WARN("Device does not support blitting from optimal tiled images");
		}
		else {
			BRISK_CORE_INFO("Device does supports blitting from optimal tiled images");
		}

		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
			BRISK_CORE_WARN("Device does not support blitting to optimal tiled images");
		}
		else {
			BRISK_CORE_INFO("Device does supports blitting to optimal tiled images");
		}

		VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint16_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint16_t>(m_RequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();

		VkPhysicalDeviceFeatures2 features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		features.features.multiDrawIndirect = VK_TRUE;
		features.features.pipelineStatisticsQuery = VK_TRUE;
		features.features.shaderInt64 = VK_TRUE;
		//features.features.shaderInt16 = VK_TRUE;

		VkPhysicalDeviceVulkan11Features features11 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		features11.storageBuffer16BitAccess = VK_TRUE;
		features11.shaderDrawParameters = VK_TRUE;

		VkPhysicalDeviceVulkan12Features features12 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.drawIndirectCount = VK_TRUE;
		features12.storageBuffer8BitAccess = VK_TRUE;
		features12.uniformAndStorageBuffer8BitAccess = VK_TRUE;
		//features12.shaderFloat16 = VK_TRUE;
		features12.shaderInt8 = VK_TRUE;
		features12.samplerFilterMinmax = VK_TRUE;
		features12.scalarBlockLayout = VK_TRUE;

		features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
		features12.runtimeDescriptorArray = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;

		deviceCreateInfo.pNext = &features;
		features.pNext = &features11;
		features11.pNext = &features12;
#if _DEBUG
		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		deviceCreateInfo.ppEnabledLayerNames = validation_layers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif
		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create logical device!");
		}

		// Get device queues
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface->GetSurface(), &presentSupport);
			if (presentSupport && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				vkGetDeviceQueue(m_Device, i, 0, &m_GraphicsQueue);
				break;
			}
		}
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
				vkGetDeviceQueue(m_Device, i, 0, &m_TransferQueue);
				break;
			}
		}
	}

	void GpuAdapterVulkan::Release() {
		vkDestroyDevice(m_Device, nullptr);
	}

	bool GpuAdapterVulkan::IsDeviceSuitable(VkPhysicalDevice device) {
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

		if (!extensionsSupported) return false;

		// Device is suitable!
		return true;
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
			case Brisk::DeferredTextures:
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = m_DeferredTexturesSet;
				write.dstBinding = bindingIndex;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = std::static_pointer_cast<TextureVulkan>(texture)->GetDescriptor();

				vkUpdateDescriptorSets(Engine::s_Application->GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), 1, &write, 0, nullptr);
				break;
			}
			case Brisk::BindlessTextures:
			{
				VkWriteDescriptorSet write{};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = m_BindlessTexturesSet;
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