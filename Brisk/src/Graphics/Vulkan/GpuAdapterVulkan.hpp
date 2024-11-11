#pragma once

#include "Engine/Renderer/GpuAdapter.hpp"
#include "SurfaceFactoryVulkan.hpp"
#include "Engine/Application.hpp"

#include <Volk/volk.h>

#include <vector>
#include <string>

namespace Brisk 
{
	class GpuAdapterVulkan : public GpuAdapter {
	public:
		enum QueueType {
			QUEUE_GRAPHICS_BIT,
			QUEUE_COMPUTE_BIT,
			QUEUE_TRANSFER_BIT,
			QUEUE_SPARSE_BINDING_BIT,
			QUEUE_PROTECTED_BIT,
			QUEUE_VIDEO_DECODE_BIT_KHR,
			QUEUE_VIDEO_ENCODE_BIT_KHR,
			QUEUE_OPTICAL_FLOW_BIT_NV,
		};

		struct Queue {
			VkQueue Handle;
			QueueType Type;
			uint32_t Index;
			uint32_t FamilyIndex;
		};

		struct QueueFamily {
			uint32_t Index;
			uint32_t QueueCount;
			std::vector<QueueType> SupportedTypes;
			bool PresentSupport;
			bool IsExplicitTransferQueue;
			bool IsExplicitComputeQueue;
		};

		struct DeviceFeatures {
			bool pRobustBufferAccess;
			bool pFullDrawIndexUint32;
			bool pImageCubeArray;
			bool pIndependentBlend;
			bool pGeometryShader;
			bool pTessellationShader;
			bool pSampleRateShading;
			bool pDualSrcBlend;
			bool pLogicOp;
			bool pMultiDrawIndirect;
			bool pDrawIndirectFirstInstance;
			bool pDepthClamp;
			bool pDepthBiasClamp;
			bool pFillModeNonSolid;
			bool pDepthBounds;
			bool pWideLines;
			bool pLargePoints;
			bool pAlphaToOne;
			bool pMultiViewport;
			bool pSamplerAnisotropy;
			bool pTextureCompressionETC2;
			bool pTextureCompressionASTC_LDR;
			bool pTextureCompressionBC;
			bool pOcclusionQueryPrecise;
			bool pPipelineStatisticsQuery;
			bool pVertexPipelineStoresAndAtomics;
			bool pFragmentStoresAndAtomics;
			bool pShaderTessellationAndGeometryPointSize;
			bool pShaderImageGatherExtended;
			bool pShaderStorageImageExtendedFormats;
			bool pShaderStorageImageMultisample;
			bool pShaderStorageImageReadWithoutFormat;
			bool pShaderStorageImageWriteWithoutFormat;
			bool pShaderUniformBufferArrayDynamicIndexing;
			bool pShaderSampledImageArrayDynamicIndexing;
			bool pShaderStorageBufferArrayDynamicIndexing;
			bool pShaderStorageImageArrayDynamicIndexing;
			bool pShaderClipDistance;
			bool pShaderCullDistance;
			bool pShaderFloat64;
			bool pShaderInt64;
			bool pShaderInt16;
			bool pShaderResourceResidency;
			bool pShaderResourceMinLod;
			bool pSparseBinding;
			bool pSparseResidencyBuffer;
			bool pSparseResidencyImage2D;
			bool pSparseResidencyImage3D;
			bool pSparseResidency2Samples;
			bool pSparseResidency4Samples;
			bool pSparseResidency8Samples;
			bool pSparseResidency16Samples;
			bool pSparseResidencyAliased;
			bool pVariableMultisampleRate;
			bool pInheritedQueries;
		};

		struct GpuRequirements {
			std::vector<QueueType> pQueueTypes;
			DeviceFeatures pFeatures;
		};

	public:
		void Init();
		void AllocatePools();
		std::vector<VkPhysicalDevice> RetrieveAvailableDevice(VkInstance instance);
		bool IsDeviceSuitable(VkPhysicalDevice device, const GpuRequirements& requirements);
		inline void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }
		void CreateLogicalDevice(const GpuRequirements& requirements);
		void Release();
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice GetDevice() { return m_Device; }
		const Queue GetGraphicsQueue() const { return m_GraphicsQueue; }
		const Queue GetTransferQueue() const { return m_TransferQueue; }
		const VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

		std::shared_ptr<SurfaceVulkan> GetSurface() { return m_Surface; }
	private:
		// Pools
		VkDescriptorPool m_DescriptorPool;

		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		Queue m_TransferQueue;
		Queue m_ComputeQueue;
		Queue m_GraphicsQueue;
		std::vector<std::vector<float>> m_QueueFamiliesPriorities;

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