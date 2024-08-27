#include "GPUContextVulkan.hpp"
//#include "Engine/Engine.hpp"
#include "Graphics/Factories/SwapchainFactory.hpp"
#include "SwapchainVulkan.hpp"
#include "Defines.h"
#include "VulkanUtilities.hpp"
#include "Graphics/ShaderManager.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

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
	VkInstance GPUContextVulkan::s_Instance;

	VkFence GPUContextVulkan::m_InFlightFence;
	VkSemaphore GPUContextVulkan::m_ImageAvailableSemaphore;
	VkSemaphore GPUContextVulkan::m_RenderFinishedSemaphore;
	std::vector<const char*> GPUContextVulkan::s_Extensions;
	std::vector<const char*> GPUContextVulkan::s_Layers;
	VkDebugUtilsMessengerCreateInfoEXT GPUContextVulkan::s_DebugCreateInfo;
	VkDebugUtilsMessengerEXT GPUContextVulkan::s_DebugMessenger;
	bool GPUContextVulkan::m_ValidationLayersFound;
	std::vector<const char*> GPUContextVulkan::s_RequiredExtensions;
	std::vector<const char*> GPUContextVulkan::s_ValidationLayers;
	VkCommandPool GPUContextVulkan::m_CommandPool;
	VkSurfaceKHR GPUContextVulkan::s_Surface;

	void GPUContextVulkan::Create(){
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

		if (glfwCreateWindowSurface(s_Instance, (GLFWwindow*)Engine::s_MainWindow->GetWindowHandle(), nullptr, &s_Surface) != VK_SUCCESS) {
			BRISK_CORE_ERROR("Failed to create window surface!");
		}

		s_GPUDevice = new GPUDeviceVulkan();
		//s_GPUDevice->Create();

		//PhysicalDevice::Details details;
		//details.Surface = s_Surface;
		//details.RequiredQueueTypes.push_back(PhysicalDevice::QueueInfo::QueueType::QUEUE_GRAPHICS_BIT);
		//details.RequiredQueueTypes.push_back(PhysicalDevice::QueueInfo::QueueType::QUEUE_TRANSFER_BIT);
		//details.RequiredFeatures.push_back(PhysicalDevice::Feature::ANISOTROPY);
		//details.RequiredFeatures.push_back(PhysicalDevice::Feature::PRESENTATION);
		//Engine::s_PhysicalDevice = new PhysicalDevice();
		//Engine::s_PhysicalDevice->Create(details);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = s_GPUDevice->GetPresentQueue()->Info.QueueFamilyIndex;

		if (vkCreateCommandPool(s_GPUDevice->GetDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	bool GPUContextVulkan::Sync() {
		vkWaitForFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);

		VkResult result = static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->AquireNextImage(UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			Engine::s_Swapchain->Release();
			static_cast<RenderPassVulkan*>(Engine::s_Renderer->GetRenderPass())->ReleaseFramebuffers();
			//delete Engine::s_Swapchain;
			Engine::s_Swapchain = SwapchainFactory::CreateSwapchain(Engine::s_MainWindow);
			Engine::s_Swapchain->Create();
			static_cast<RenderPassVulkan*>(Engine::s_Renderer->GetRenderPass())->CreateFramebuffers();
			return true;
		}

		vkResetFences(s_GPUDevice->GetDevice(), 1, &m_InFlightFence);
		return false;
	}

	void GPUContextVulkan::WaitDeviceIdle() {
		vkDeviceWaitIdle(s_GPUDevice->GetDevice());
	}

	void GPUContextVulkan::Submit(RenderPassVulkan* renderpass) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkCommandBuffer cmdBufer = renderpass->GetCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBufer;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(s_GPUDevice->GetGraphicsQueue()->Queue_, 1, &submitInfo, m_InFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &m_ImageIndex;

		vkQueuePresentKHR(s_GPUDevice->GetPresentQueue()->Queue_, &presentInfo);
	}

	void GPUContextVulkan::PrepreFrame(VkCommandBuffer commandBuffer) {
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
	}

	void GPUContextVulkan::Draw(VkCommandBuffer commandBuffer, BufferVulkan buffer) {
		const VkBuffer vertexBuffers[] = { buffer.Get() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(commandBuffer, buffer.GetData().size(), 1, 0, 0);
	}

	void GPUContextVulkan::CreateSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(s_GPUDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(s_GPUDevice->GetDevice(), &fenceInfo, nullptr, &m_InFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	void GPUContextVulkan::ReleasePools() {
		// TODO: Should not get freed here
		vkDestroySemaphore(s_GPUDevice->GetDevice(), m_ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(s_GPUDevice->GetDevice(), m_RenderFinishedSemaphore, nullptr);
		vkDestroyFence(s_GPUDevice->GetDevice(), m_InFlightFence, nullptr);
		vkDestroyCommandPool(s_GPUDevice->GetDevice(), m_CommandPool, nullptr);
	}

	void GPUContextVulkan::Release() {
		vkDestroyDebugUtilsMessengerEXT(s_Instance, s_DebugMessenger, nullptr);
		vkDestroyInstance(s_Instance, nullptr);
	}
}