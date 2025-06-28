#pragma once

// INCLUDES
#include "Engine/Renderer/GpuAdapter.hpp"
#include "SurfaceFactoryVulkan.hpp"
#include "Engine/Application.hpp"
//-------------------------------
#include <volk.h>
#include "vk_mem_alloc.h"
//--------------------
#include <vector>
#include <string>
//---------------

namespace Brisk 
{
	// Predefined descriptor set values
	constexpr uint32_t SET_MVP = 0;
	constexpr uint32_t SET_LIGHTS = 1;
	constexpr uint32_t SET_DEFERRED_TEXTURES = 2;
	constexpr uint32_t SET_BINDLESS = 3;
	constexpr uint32_t SET_MATERIALS = 4;
	//

	class GpuAdapterVulkan : public GpuAdapter {
	public:
		virtual void Init() override;
		void AllocatePools();
		std::vector<VkPhysicalDevice> RetrieveAvailableDevice(VkInstance instance);
		bool IsDeviceSuitable(VkPhysicalDevice device/*, const GpuRequirements& requirements*/);
		inline void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }
		void CreateLogicalDevice(/*const GpuRequirements& requirements*/);
		void Release();
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice& GetDevice() { return m_Device; }
		VkCommandPool GetCommandPool() { return m_CommandPool; }
		VmaAllocator GetVmaAllocator() { return m_VmaAllocator; }
		const VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		const VkQueue GetTransferQueue() const { return m_TransferQueue; }
		const uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }
		const uint32_t GetTransferQueueFamily() const { return m_TransferQueueFamily; }
		const VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }
		const VkInstance GetInstance() const { return m_Instance; }

		virtual void AddResource(GpuDescriptorResourceType type, std::shared_ptr<Texture> texture, std::shared_ptr<Buffer> buffer, int bindingIndex) override;

		std::shared_ptr<SurfaceVulkan> GetSurface() { return m_Surface; }

	public:
		VkDescriptorSetLayout m_DummyDescriptorLayout;
		VkDescriptorSetLayout m_MVPDescriptorLayout;
		VkDescriptorSetLayout m_LightsDescriptorLayout;
		VkDescriptorSetLayout m_DeferredTexturesDescriptorLayout;
		VkDescriptorSetLayout m_BindlessDescriptorLayout;
		VkDescriptorSetLayout m_MaterialsDescriptorLayout;

		// Descriptor stuff ------------------------------------//
		VkDescriptorPool m_DescriptorPool;						//
		VkDescriptorPool m_BindlessDescriptorPool;				//
																//
		// Projection Matrix									//
		// View Matrix											//
		// Camera Position Matrix								//
		VkDescriptorSet m_MVPUBOSet;							//
		// Scene Lights											//
		VkDescriptorSet m_SceneLightsSet;						//
		// Textures												//
		// Shadow map, 											//
		// Deferred textures, 									//
		VkDescriptorSet m_DeferredTexturesSet;					//
		// Bindless textures									//
		VkDescriptorSet m_BindlessTexturesSet;					//
		// Materials											//
		VkDescriptorSet m_MaterialsSet;							//
		//------------------------------------------------------//

	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		VkQueue m_TransferQueue;
		VkQueue m_GraphicsQueue;

		VmaAllocator m_VmaAllocator;

		uint32_t m_GraphicsQueueFamily;
		uint32_t m_TransferQueueFamily;

		VkCommandPool m_CommandPool;

		///
		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		std::vector<const char*> m_Extensions;
		std::vector<const char*> m_Layers;
		std::vector<const char*> m_RequiredExtensions;
		std::vector<const char*> m_ValidationLayers;
		VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		VkDebugUtilsMessengerEXT s_DebugMessenger;
		bool m_ValidationLayersFound;

		std::shared_ptr<SurfaceVulkan> m_Surface;

		/// <summary>
		/// Vulkan handles
		/// </summary>
		VkInstance m_Instance;
	};
}