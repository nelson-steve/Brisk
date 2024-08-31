#pragma once

#include "Engine/WindowBase.hpp"
#include "Graphics/Vulkan/RenderpassVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuDeviceVulkan.hpp"
#include "SurfaceFactoryVulkan.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk 
{
	class GpuContextVulkan {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		void Create();
		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		void Release();

		bool CreateDevice(const GpuDeviceVulkan::GpuRequirements& requirements);
		void WaitDeviceIdle();

		/// <summary>
		/// Getters for Vulkan handles
		/// </summary>
		static VkInstance GetInstance() { return s_Instance; }

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
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
		static std::vector<const char*> s_RequiredExtensions;
		static std::vector<const char*> s_ValidationLayers;
		static VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static bool m_ValidationLayersFound;

		static GpuDeviceVulkan* s_GPUDevice;
		static SurfaceVulkan* s_Surface;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class GraphicsPipelineVulkan;
		friend class RenderPassVulkan;
		friend class GpuDeviceVulkan;
		friend class SwapchainVulkan;
		friend class VulkanUtilities;
		friend class RendererVulkan;
		friend class CommandBufferVulkan;
		friend class BufferVulkan;
	};
}