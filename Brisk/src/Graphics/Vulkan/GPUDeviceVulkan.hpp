#pragma once

#include "GPUContextVulkan.hpp"

#include <Volk/volk.h>

#include <vector>
#include <string>

namespace Brisk 
{
	struct Queue;

	class GpuDeviceVulkan {
	public:
		//struct QueueInfo {
		//	enum QueueType {
		//		QUEUE_GRAPHICS_BIT,
		//		QUEUE_COMPUTE_BIT,
		//		QUEUE_TRANSFER_BIT,
		//		QUEUE_SPARSE_BINDING_BIT,
		//		QUEUE_PROTECTED_BIT,
		//		QUEUE_VIDEO_DECODE_BIT_KHR,
		//		QUEUE_VIDEO_ENCODE_BIT_KHR,
		//		QUEUE_OPTICAL_FLOW_BIT_NV,
		//	} m_QueueType;
		//	float Priority;
		//	uint32_t QueueIndex;
		//	uint32_t QueueFamilyIndex;
		//	uint32_t QueueCount;
		//	bool PresentSupport = false;
		//	std::vector<QueueType> SupportedQueueTypes;
		//};
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

		struct Details {
		public:
			VkSurfaceKHR Surface = VK_NULL_HANDLE;
			VkBool32 SamplerAnisotropy = VK_TRUE;
		};

	public:
		void Create();
		std::vector<VkPhysicalDevice> RetrieveAvailableDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device, const GpuRequirements& requirements);
		inline void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { m_PhysicalDevice = physicalDevice; }
		void CreateLogicalDevice(const GpuRequirements& requirements);
		void Release();
		bool CreateQueueFamilies(VkPhysicalDevice device, const Details& details);
		//QueueFamilyIndices GetQueueFamilies() { return m_Indices; }
		VkQueueFlags QueueTypeToVulkanType(QueueType type);
		const std::string& QueueTypeToString(QueueType type);
		void PrintQueueFlags(VkQueueFlags flags);
		void GetSupportedQueueTypes(VkQueueFlags flags, std::vector<QueueType>& ref);
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		//const std::vector<PhysicalDevice::QueueInfo> RetrieveCommonQueues();
		const std::vector<std::vector<float>> RetreivePriorities();
		VkDevice GetDevice() { return m_Device; }
		const Queue* GetGraphicsQueue() const { return m_GraphicsQueue; }
		const Queue* GetPresentQueue() const { return m_PresentQueue; }
	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		Queue* m_PresentQueue;
		Queue* m_GraphicsQueue;
		std::vector<std::vector<float>> m_QueueFamiliesPriorities;
	};

	struct Queue {
		VkQueue Queue_;
		//PhysicalDevice::QueueInfo Info;
	};
}