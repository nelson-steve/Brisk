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

	VkFence GraphicsDeviceVulkan::m_InFlightFence;
	VkSemaphore GraphicsDeviceVulkan::m_ImageAvailableSemaphore;
	VkSemaphore GraphicsDeviceVulkan::m_RenderFinishedSemaphore;
	std::vector<const char*> GraphicsDeviceVulkan::s_Extensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_Layers;
	VkDebugUtilsMessengerCreateInfoEXT GraphicsDeviceVulkan::s_DebugCreateInfo;
	VkDebugUtilsMessengerEXT GraphicsDeviceVulkan::s_DebugMessenger;
	bool GraphicsDeviceVulkan::m_ValidationLayersFound;
	std::vector<const char*> GraphicsDeviceVulkan::s_RequiredExtensions;
	std::vector<const char*> GraphicsDeviceVulkan::s_ValidationLayers;
	VkCommandPool GraphicsDeviceVulkan::m_CommandPool;
	VkCommandBuffer GraphicsDeviceVulkan::m_CommandBuffer;

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

	void GraphicsDeviceVulkan::CreateCommandPoolAndBuffer() {

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//poolInfo.queueFamilyIndex = Engine::s_PhysicalDevice->GetQueueFamilies().PresentIndex;

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

	void GraphicsDeviceVulkan::Draw() {
		vkWaitForFences(Engine::s_PhysicalDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(Engine::s_PhysicalDevice->GetDevice(), 1, &m_InFlightFence);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(Engine::s_PhysicalDevice->GetDevice(), 
			static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(m_CommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
		RecordCommandBuffer(m_CommandBuffer, imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(Engine::s_PhysicalDevice->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(Engine::s_PhysicalDevice->GetPresentQueue(), &presentInfo);
	}

	void GraphicsDeviceVulkan::WaitDeviceIdle() {
		vkDeviceWaitIdle(Engine::s_PhysicalDevice->GetDevice());
	}

	void GraphicsDeviceVulkan::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//renderPassInfo.renderPass = m_GraphicsPipeline->GetRenderPass();
		renderPassInfo.framebuffer = m_Renderpass->GetFramebuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::s_Renderer->GetDefaultGraphicsPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentWidth());
		viewport.height = static_cast<float>(static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtentHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void GraphicsDeviceVulkan::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(Engine::s_PhysicalDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(Engine::s_PhysicalDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(Engine::s_PhysicalDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}

	}

	void GraphicsDeviceVulkan::Release() {
		delete m_Renderpass;

		vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugMessenger, nullptr);
		vkDestroyInstance(s_Instance, nullptr);
	}
}