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
		appInfo.apiVersion = VK_API_VERSION_1_4;

		m_InstanceExtensions = UtilitiesVulkan::GetRequiredExtensions();
		m_ValidationLayersFound = false;
#if _DEBUG
		m_ValidationLayersFound = UtilitiesVulkan::CheckValidationLayerSupport(m_ValidationLayers);
		if (!m_ValidationLayersFound) {
			BRISK_APP_ERROR("Validation layers not found");
		}
#endif
		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_InstanceExtensions.data();
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

		m_DeviceExtensions =
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		m_DeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

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
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
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
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_VERTEX_BIT, 
				GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT,
				GPUResource::ShaderStageAccess::SHADER_STAGE_MESH_BIT_EXT });
			layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT,  GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(3, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_MESH_BIT_EXT });
			layout->AddBinding(4, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_MESH_BIT_EXT });
			layout->AddBinding(5, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_IMAGE,  { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->Init();
			m_FrameGlobalDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(3, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(4, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(5, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, { GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->Init();
			m_PerMeshDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		{
			std::shared_ptr<DescriptorLayout> layout = DescriptorLayout::Create();
			layout->AddBinding(0, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT, GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(1, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_UNIFORM_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT, GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(2, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT, GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(3, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT, GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(4, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT, GPUResource::ShaderStageAccess::SHADER_STAGE_FRAGMENT_BIT });
			layout->AddBinding(5, 1, GPUResource::ResourceType::DESCRIPTOR_TYPE_STORAGE_BUFFER, { GPUResource::ShaderStageAccess::SHADER_STAGE_COMPUTE_BIT });
			layout->Init();
			m_ClusteredLightingDescriptorLayout = std::static_pointer_cast<DescriptorLayoutVulkan>(layout)->GetLayout();
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_FrameGlobalDescriptorLayout;
		if (vkAllocateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_GlobalSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		allocInfo.pSetLayouts = &m_PerMeshDescriptorLayout;
		if (vkAllocateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_PerMeshSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		allocInfo.pSetLayouts = &m_ClusteredLightingDescriptorLayout;
		if (vkAllocateDescriptorSets(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &allocInfo, &m_ClusteredLightingSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		// Creating Graphics command pool
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = m_GraphicsQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}

		// Creating Compute command pool
		poolInfo.queueFamilyIndex = m_ComputeQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &m_ComputeCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}

		// Creating Transfer command pool
		poolInfo.queueFamilyIndex = m_TransferQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(Application::GetGpuAdapter()->GetDevice<GpuAdapterVulkan>()->GetDevice(), &poolInfo, nullptr, &m_TransferCommandPool) != VK_SUCCESS) {
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
		poolInfo.maxSets = 1024;

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
				throw std::runtime_error("Failed to create bindless descriptor pool!");
			}

			std::vector<VkDescriptorSetLayoutBinding> bindings;

			VkDescriptorSetLayoutBinding imageSamplerBinding{};
			imageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageSamplerBinding.descriptorCount = maxBindlessResources;
			imageSamplerBinding.binding = 0;
			imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
			imageSamplerBinding.pImmutableSamplers = nullptr;
			bindings.push_back(imageSamplerBinding);

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
				throw std::runtime_error("Failed to create bindless descriptor pool!");
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

			if (vkAllocateDescriptorSets(m_Device, &allocInfo, &m_BindlessSet) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create bindless descriptor pool!");
			}
		}
	}

	void GpuAdapterVulkan::CreateLogicalDevice() {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> skipFamilies;
		float queuePriority = 1.0f;
		// Find dedicated graphics queue family
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
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

		// Find dedicated compute queue family
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (std::find(skipFamilies.begin(), skipFamilies.end(), i) != skipFamilies.end())
				continue;
			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT     &&
				!(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				VkDeviceQueueCreateInfo transferQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				m_ComputeQueueFamily = i;
				transferQueueCreateInfo.queueFamilyIndex = i;
				transferQueueCreateInfo.queueCount = 1;
				transferQueueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(transferQueueCreateInfo);
				break;
			}
		}

		// Find dedicated transfer queue family
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (std::find(skipFamilies.begin(), skipFamilies.end(), i) != skipFamilies.end())
				continue;
			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
				!(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
				!(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
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
		deviceCreateInfo.enabledExtensionCount = static_cast<uint16_t>(m_DeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		VkPhysicalDeviceFeatures2 features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		features.features.multiDrawIndirect = VK_TRUE;
		features.features.pipelineStatisticsQuery = VK_TRUE;
		features.features.shaderInt16 = VK_TRUE;
		features.features.shaderInt64 = VK_TRUE;
		features.features.samplerAnisotropy = VK_TRUE;
		features.features.geometryShader = VK_TRUE;
		features.features.fragmentStoresAndAtomics = VK_TRUE;

		VkPhysicalDeviceVulkan11Features features11 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		features11.storageBuffer16BitAccess = VK_TRUE;
		features11.shaderDrawParameters = VK_TRUE;

		VkPhysicalDeviceVulkan12Features features12 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.drawIndirectCount = VK_TRUE;
		features12.storageBuffer8BitAccess = VK_TRUE;
		features12.uniformAndStorageBuffer8BitAccess = VK_TRUE;
		features12.shaderFloat16 = VK_TRUE;
		features12.shaderInt8 = VK_TRUE;
		features12.samplerFilterMinmax = VK_TRUE;
		features12.scalarBlockLayout = VK_TRUE;

		// for raytracing
		//features12.bufferDeviceAddress = VK_TRUE;

		// Bindless features
		features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
		features12.runtimeDescriptorArray = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;

		VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.dynamicRendering = true;
		features13.synchronization2 = true;
		features13.maintenance4 = true;
		features13.shaderDemoteToHelperInvocation = true;

		VkPhysicalDeviceVulkan14Features features14 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES };
		features14.maintenance5 = true;
		features14.maintenance6 = true;
		features14.pushDescriptor = true;

		VkPhysicalDeviceMeshShaderFeaturesEXT featuresMesh = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
		featuresMesh.taskShader = true;
		featuresMesh.meshShader = true;

		// Raytracing features
		//VkPhysicalDeviceRayQueryFeaturesKHR featuresRayQueries = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
		//featuresRayQueries.rayQuery = true;
		//VkPhysicalDeviceAccelerationStructureFeaturesKHR featuresAccelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		//featuresAccelerationStructure.accelerationStructure = true;

		deviceCreateInfo.pNext = &features;
		features.pNext = &features11;
		features11.pNext = &features12;
		features12.pNext = &features13;
		features13.pNext = &features14;
		features14.pNext = &featuresMesh;
#if _DEBUG
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
		deviceCreateInfo.enabledLayerCount = 0;
#endif
		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create logical device!");
		}

		BRISK_CORE_ASSERT(m_GraphicsQueueFamily != -1);
		BRISK_CORE_ASSERT(m_ComputeQueueFamily != -1);
		BRISK_CORE_ASSERT(m_TransferQueueFamily != -1);

		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_ComputeQueueFamily, 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, m_TransferQueueFamily, 0, &m_TransferQueue);
	}

	void GpuAdapterVulkan::Release() {
		vmaDestroyAllocator(m_VmaAllocator);

		m_Surface->Release(m_Instance);

		vkDestroyDevice(m_Device, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void GpuAdapterVulkan::ReleasePools() {
		vkDestroyDescriptorSetLayout(m_Device, m_DummyDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_FrameGlobalDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_BindlessDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_PerMeshDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_ClusteredLightingDescriptorLayout, nullptr);

		vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_ComputeCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);

		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorPool(m_Device, m_BindlessDescriptorPool, nullptr);
	}

	void GpuAdapterVulkan::WaitIdle() {
		vkDeviceWaitIdle(m_Device);
	}

	bool GpuAdapterVulkan::IsDeviceSuitable(VkPhysicalDevice device) {
		std::vector<const char*>& extensions = m_DeviceExtensions;
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
		BRISK_CORE_ASSERT(bindless_supported); // Bindless rendering is a dependency for now

		if (!extensionsSupported) return false;

		// Device is suitable!
		return true;
	}
}