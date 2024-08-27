#pragma once

#include "Engine/WindowBase.hpp"
#include "Graphics/Vulkan/RenderpassVulkan.hpp"
#include "BufferVulkan.hpp"

#include <Volk/volk.h>

#include <vector>
#include "GPUDeviceVulkan.hpp"

namespace Brisk 
{
	/// <summary>
	/// Forward declarations
	/// </summary>
	class Swapchain;

	class GPUContextVulkan {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		void Create();

		bool Sync();
		void WaitDeviceIdle();

		void CreateSyncObjects();
		void PrepreFrame(VkCommandBuffer commandBuffer);
		void Submit(RenderPassVulkan* renderpass);
		void Draw(VkCommandBuffer commandBuffer, BufferVulkan buffer);
		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		void Release();
		void ReleasePools();

		/// <summary>
		/// Getters for Vulkan handles
		/// </summary>
		static VkInstance GetInstance() { return s_Instance; }
		static VkCommandPool GetCommandPool() { return m_CommandPool; }
		static VkSurfaceKHR GetSurface() { return s_Surface; }

		uint32_t GetImageIndex() { return m_ImageIndex; }

		static std::vector<const char*>& GetRequiredExtenstions() { return s_RequiredExtensions; }
		static std::vector<const char*>& GetValidationLayers() { return s_ValidationLayers; }
	protected:
		/// <summary>
		/// Vulkan handles
		/// </summary>
		static VkInstance s_Instance;

		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		static VkCommandPool m_CommandPool;
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
		static std::vector<const char*> s_RequiredExtensions;
		static std::vector<const char*> s_ValidationLayers;
		static VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static bool m_ValidationLayersFound;

		static GPUDeviceVulkan* s_GPUDevice;
		static GraphicsPipelineVulkan* s_GraphicsPipeline;
		

		static VkFence m_InFlightFence;
		static VkSemaphore m_ImageAvailableSemaphore;
		static VkSemaphore m_RenderFinishedSemaphore;

		static VkSurfaceKHR s_Surface;
		static GPUDeviceVulkan* s_GPUDevice;

		uint32_t m_ImageIndex;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class VulkanUtilities;
		friend class GPUDeviceVulkan;
		friend class GraphicsPipelineVulkan;
	};
}