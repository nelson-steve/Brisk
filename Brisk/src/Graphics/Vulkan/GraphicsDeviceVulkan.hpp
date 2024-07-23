#pragma once

#include "PhysicalDevice.hpp"
#include "Engine/WindowBase.hpp"
#include "Graphics/GPUDevice.hpp"
#include "GraphicsPipelineVulkan.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk 
{
	/// <summary>
	/// Forward declarations
	/// </summary>
	class Swapchain;

	class GraphicsDeviceVulkan : public GPUDevice {
	public:
		/// <summary>
		/// Create Vulkan Device
		/// </summary>
		virtual void Create() override;

		virtual void SetupGraphicsPipeline(std::vector<ShaderInfo> shaders) override;
		virtual void ReleaseGraphicsPipeline() override;

		void CreateCommandPoolAndBuffer();
		void RecordCommandBuffer();

		/// <summary>
		/// Release all Vulkan resources cleanly
		/// </summary>
		virtual void Release() override;


		/// <summary>
		/// Getters for Vulkan handles
		/// </summary>
		static VkInstance GetInstance() { return s_Instance; }
		static std::vector<const char*> GetRequiredExtenstions() { return s_RequiredExtensions; }
		static std::vector<const char*> GetValidationLayers() { return s_ValidationLayers; }

	public:
		// TODO: This should not exist here I think
		static GraphicsPipelineVulkan* m_GraphicsPipeline;
	private:
		/// <summary>
		/// Vulkan handles
		/// </summary>
		static VkInstance s_Instance;

		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		static VkCommandPool m_CommandPool;
		static VkCommandBuffer m_CommandBuffer;
		static std::vector<const char*> s_Extensions;
		static std::vector<const char*> s_Layers;
		static std::vector<const char*> s_RequiredExtensions;
		static std::vector<const char*> s_ValidationLayers;
		static VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		static VkDebugUtilsMessengerEXT s_DebugMessenger;
		static bool m_ValidationLayersFound;

		/// <summary>
		/// Friend class declaration
		/// </summary>
		friend class VulkanUtilities;
	};
}