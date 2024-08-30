#pragma once

#include "Engine/WindowBase.hpp"
#include "Graphics/Vulkan/RenderpassVulkan.hpp"
#include "BufferVulkan.hpp"

#include <Volk/volk.h>

#include <vector>
#include "GpuDeviceVulkan.hpp"
#include "SurfaceFactoryVulkan.hpp"

namespace Brisk 
{
	/// <summary>
	/// Forward declarations
	/// </summary>
	class Swapchain;

	class GpuContextVulkan {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		void Create();
		bool CreateDevice(const GpuDeviceVulkan::GpuRequirements& requirements);


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

		static uint32_t GetImageIndex() { return m_ImageIndex; }

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

		static GpuDeviceVulkan* s_GPUDevice;
		static GraphicsPipelineVulkan* s_GraphicsPipeline;
		static RenderPassVulkan* s_RenderPass;
		
		static VkFence m_InFlightFence;
		static VkSemaphore m_ImageAvailableSemaphore;
		static VkSemaphore m_RenderFinishedSemaphore;

		static SurfaceVulkan* s_Surface;
		static GpuDeviceVulkan* s_GPUDevice;

		static uint32_t m_ImageIndex;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class GpuDeviceVulkan;
		friend class GraphicsPipelineVulkan;
		friend class RenderPassVulkan;
		friend class SwapchainVulkan;
		friend class VulkanUtilities;
		friend class RendererVulkan;
	};
}