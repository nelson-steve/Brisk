#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"
#include "SwapchainVulkan.hpp"
#include "Defines.h"
#include "VulkanUtilities.hpp"
#include "Graphics/ShaderManager.hpp"

namespace Brisk 
{
	static void CheckAvailableExtensions() {
		VkResult result;

		/*
		 * From the link above:
		 * If `pProperties` is NULL, then the number of extensions properties
		 * available is returned in `pPropertyCount`.
		 *
		 * Basically, gets the number of extensions.
		 */
		uint32_t count = 0;
		result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		if (result != VK_SUCCESS) {
			// Throw an exception or log the error
		}

		std::vector<VkExtensionProperties> extensionProperties(count);

		// Get the extensions
		result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
		if (result != VK_SUCCESS) {
			assert(false);
		}

		//std::set<std::string> extensions;
		std::cout << "Availble Instance Extension:" << std::endl;
		for (auto& extension : extensionProperties) {
			std::cout << extension.extensionName << std::endl;
		}

		std::vector<VkLayerProperties> layers;

		count = 0;
		result = vkEnumerateInstanceLayerProperties(&count, nullptr);
		if (result != VK_SUCCESS) {
			assert(false);
		}

		layers.resize(count);
		result = vkEnumerateInstanceLayerProperties(&count, layers.data());
		if (result != VK_SUCCESS) {
			assert(false);
		}
		std::cout << "Availble Instance Layers:" << std::endl;
		for (auto& layer : layers) {
			std::cout << layer.layerName << std::endl;
		}

	}

	/// <summary>
	/// Static memebers declarations
	/// </summary>
	VkInstance GraphicsDeviceVulkan::s_Instance;

	GraphicsPipelineVulkan* GraphicsDeviceVulkan::m_GraphicsPipeline;
	std::vector<const char*> GraphicsDeviceVulkan::s_Extensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_Layers;
	VkDebugUtilsMessengerCreateInfoEXT GraphicsDeviceVulkan::s_DebugCreateInfo;
	VkDebugUtilsMessengerEXT GraphicsDeviceVulkan::s_DebugMessenger;
	bool GraphicsDeviceVulkan::m_ValidationLayersFound;
	std::vector<const char*> GraphicsDeviceVulkan::s_RequiredExtensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_ValidationLayers;
	VkCommandPool m_CommandPool;
	VkCommandBuffer m_CommandBuffer;
	void GraphicsDeviceVulkan::Create(){
		volkInitialize();

		s_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Engine::s_EngineInfo.EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		s_Extensions = VulkanUtilities::GetRequiredExtensions();
		m_ValidationLayersFound = false;
#if _DEBUG
		m_ValidationLayersFound = VulkanUtilities::CheckValidationLayerSupport(s_ValidationLayers);
		if (!m_ValidationLayersFound) {
			BRISK_APP_ERROR("Validation layers not found");
		}
#endif
		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(s_Extensions.size());
		createInfo.ppEnabledExtensionNames = s_Extensions.data();
#if _DEBUG
		createInfo.enabledLayerCount =
			m_ValidationLayersFound ? static_cast<uint32_t>(s_ValidationLayers.size()) : 0;
		createInfo.ppEnabledLayerNames =
			m_ValidationLayersFound ? s_ValidationLayers.data() : nullptr;

		VulkanUtilities::PopulateDebugMessengerCreateInfo();
		createInfo.pNext = &s_DebugCreateInfo;
#endif
		VK_LOG(vkCreateInstance(&createInfo, nullptr, &s_Instance),
			"Failed to create Vulkan instance");

		volkLoadInstance(s_Instance);

#if _DEBUG
		VkResult result = VulkanUtilities::CreateDebugUtilsMessengerEXT();
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			BRISK_APP_ERROR("Debug Utils extension not present");
		}
		else if(result != VK_SUCCESS) {
			BRISK_APP_ERROR("Failed to create debug messenger");
		}
#endif

		s_RequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	void GraphicsDeviceVulkan::SetupGraphicsPipeline(std::vector<ShaderInfo> shaders) {
		std::vector<ShaderModule> modules;
		for (ShaderInfo shader : shaders) {
			modules.push_back(ShaderManager::CreateShaderModule(shader));
		}

		m_GraphicsPipeline = new GraphicsPipelineVulkan();
		m_GraphicsPipeline->Create(modules);

		dynamic_cast<SwapchainVulkan*>(Engine::m_Swapchain)->CreateFramebuffer();
	}

	void GraphicsDeviceVulkan::ReleaseGraphicsPipeline() {
		m_GraphicsPipeline->Release();
	}

	void GraphicsDeviceVulkan::CreateCommandPoolAndBuffer() {

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Engine::s_PhysicalDevice->GetQueueFamilies().PresentIndex;

		if (vkCreateCommandPool(Engine::s_PhysicalDevice->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(Engine::s_PhysicalDevice->GetDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void GraphicsDeviceVulkan::RecordCommandBuffer() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void GraphicsDeviceVulkan::Release() {
		vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugMessenger, nullptr);
		vkDestroyInstance(s_Instance, nullptr);
	}
}