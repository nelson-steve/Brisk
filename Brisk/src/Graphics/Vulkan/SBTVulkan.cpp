#include "pch.hpp"
#include "SBTVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"

namespace Brisk
{
	uint32_t shaderGroupHandleSize = 32;
	uint32_t shaderGroupBaseAlignment = 64;
	uint32_t shaderGroupHandleAlignment = 4;
	uint32_t groupCount = 3;

	void SBTVulkan::Init(std::shared_ptr<Pipeline> pipeline) {
		size_t dataSize = shaderGroupHandleSize * groupCount;
		std::vector<uint8_t> m_shaderHandles(dataSize);
		VkPipeline vkPipeline = std::static_pointer_cast<PipelineVulkan>(pipeline)->GetPipeline();
		if (vkGetRayTracingShaderGroupHandlesKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), vkPipeline, 0, groupCount, dataSize, m_shaderHandles.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to get ray tracing shader group handle");
		}

		auto alignUp = [](uint32_t size, uint32_t alignment) { return (size + alignment - 1) & ~(alignment - 1); };

		uint32_t raygenStride = alignUp(shaderGroupHandleSize, shaderGroupHandleAlignment);
		uint32_t missStride = alignUp(shaderGroupHandleSize, shaderGroupHandleAlignment);
		uint32_t hitStride = alignUp(shaderGroupHandleSize, shaderGroupHandleAlignment);
		uint32_t callableStride = 0;

		uint32_t missCount = 1;

		uint32_t raygenSize = raygenStride * 1;
		uint32_t missSize = missStride * missCount;
		uint32_t hitSize = hitStride * 1;
		uint32_t callableSize = 0;

		uint32_t raygenOffset = 0;
		uint32_t missOffset = alignUp(raygenSize, shaderGroupBaseAlignment);
		uint32_t hitOffset = alignUp(missOffset + missSize, shaderGroupBaseAlignment);
		uint32_t callableOffset = alignUp(hitOffset + hitSize, shaderGroupBaseAlignment);

		size_t bufferSize = callableOffset + callableSize;

		buffer = Buffer::Create();
		BufferDesc bufferDesc{};
		bufferDesc.p_Size = bufferSize;
		bufferDesc.p_Usage = Core::BufferUsage::ShaderDeviceAddress | Core::BufferUsage::ShaderBindingTable;
		bufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		bufferDesc.p_GpuMapped = true;
		bufferDesc.p_Persistant = true;
		buffer->Init(bufferDesc);

		VkDeviceAddress sbtDeviceAddress = std::static_pointer_cast<BufferVulkan>(buffer)->GetDeviceAddress();

		uint8_t* pData = static_cast<uint8_t*>(std::static_pointer_cast<BufferVulkan>(buffer)->GetMapped());

		// Raygen
		memcpy(pData + raygenOffset,
			m_shaderHandles.data() + 0 * shaderGroupHandleSize,
			shaderGroupHandleSize);

		// Miss #0
		memcpy(pData + missOffset,
			m_shaderHandles.data() + 1 * shaderGroupHandleSize,
			shaderGroupHandleSize);

		// Hit
		memcpy(pData + hitOffset,
			m_shaderHandles.data() + 2 * shaderGroupHandleSize,
			shaderGroupHandleSize);


		rayGen.deviceAddress = sbtDeviceAddress + raygenOffset;
		rayGen.stride = raygenStride;
		rayGen.size = raygenSize;

		// Miss shader
		miss.deviceAddress = sbtDeviceAddress + missOffset;
		miss.stride = missStride;
		miss.size = missSize;

		// Hit shader
		hit.deviceAddress = sbtDeviceAddress + hitOffset;
		hit.stride = hitStride;
		hit.size = hitSize;

		// Callable shaders
		callable.deviceAddress = 0;
		callable.stride = 0;
		callable.size = 0;
	}
}
