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
	class GpuAdapterVulkan : public GpuAdapter {
	public:
		virtual void Init() override;
		void AllocatePools();
		std::vector<VkPhysicalDevice> RetrieveAvailableDevice(VkInstance instance);
		bool IsDeviceSuitable(VkPhysicalDevice device/*, const GpuRequirements& requirements*/);
		inline void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }
		void CreateLogicalDevice(/*const GpuRequirements& requirements*/);
		virtual void Release() override;
		virtual void ReleasePools() override;

		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice& GetDevice() { return m_Device; }
		VkCommandPool GetGraphicsCommandPool() const { return m_GraphicsCommandPool; }
		VkCommandPool GetComputeCommandPool() const { return m_ComputeCommandPool; }
		VkCommandPool GetTransferCommandPool() const { return m_TransferCommandPool; }
		VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetComputeQueue() const { return m_ComputeQueue; }
		VkQueue GetTransferQueue() const { return m_TransferQueue; }
		uint32_t GetGraphicsQueueFamily() const { return m_GraphicsQueueFamily; }
		uint32_t GetTransferQueueFamily() const { return m_TransferQueueFamily; }
		VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }
		VkInstance GetInstance() const { return m_Instance; }

		virtual void WaitIdle() override;

		std::shared_ptr<SurfaceVulkan> GetSurface() { return m_Surface; }

	public:
		VkDescriptorSetLayout m_DummyDescriptorLayout;
		VkDescriptorSetLayout m_FrameGlobalDescriptorLayout;
		VkDescriptorSetLayout m_BindlessDescriptorLayout;
		VkDescriptorSetLayout m_PerMeshDescriptorLayout;
		VkDescriptorSetLayout m_ClusteredLightingDescriptorLayout;

		// Descriptor stuff ------------------------------------//
		VkDescriptorPool m_DescriptorPool;						//
		VkDescriptorPool m_BindlessDescriptorPool;				//
																//
		// Projection Matrix									//
		// View Matrix											//
		// Camera Position Matrix								//
		// Materials											//
		VkDescriptorSet m_GlobalSet;							//
		// Scene Lights											//
		VkDescriptorSet m_BindlessSet;							//
		// Deferred textures, 									//
		VkDescriptorSet m_PerMeshSet;							//
		// Clustered Lighting								    //
		VkDescriptorSet m_ClusteredLightingSet;					//
		//------------------------------------------------------//

	private:
		/// <summary>
		/// Vulkan main handles
		/// </summary>
		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;

		/// <summary>
		/// Vulkan queue handles
		/// </summary>
		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_TransferQueue;

		VmaAllocator m_VmaAllocator;

		/// <summary>
		/// Vulkan queue family indexes cached to create dedicated queues
		/// </summary>
		int32_t m_GraphicsQueueFamily = -1;
		int32_t m_ComputeQueueFamily = -1;
		int32_t m_TransferQueueFamily = -1;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;
		VkCommandPool m_TransferCommandPool;

		///
		/// <summary>
		/// Vulkan helper variables
		/// </summary>
		std::vector<const char*> m_InstanceExtensions;
		std::vector<const char*> m_Layers;
		std::vector<const char*> m_DeviceExtensions;
		std::vector<const char*> m_ValidationLayers;
		VkDebugUtilsMessengerCreateInfoEXT s_DebugCreateInfo;
		VkDebugUtilsMessengerEXT s_DebugMessenger;
		bool m_ValidationLayersFound;

		std::shared_ptr<SurfaceVulkan> m_Surface;
	};
}