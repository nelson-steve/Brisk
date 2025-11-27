#include "SBTVulkan.hpp"
#include "BufferVulkan.hpp"

namespace Brisk
{
	uint32_t shaderGroupHandleSize = 32;
	uint32_t shaderGroupBaseAlignment = 64;
	uint32_t shaderGroupHandleAlignment = 4;

	void SBTVulkan::Init(Type type, uint32_t count) {
		buffer = Buffer::Create();
		BufferDesc bufferDesc{};
		bufferDesc.p_Size = shaderGroupHandleSize * count;
		bufferDesc.p_Usage = Core::BufferUsage::ShaderDeviceAddress | Core::BufferUsage::ShaderBindingTable;
		bufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU;
		bufferDesc.p_Persistant = true;
		buffer->Init(bufferDesc);
;
		uint32_t handleSizeAligned = (shaderGroupHandleSize + shaderGroupHandleAlignment - 1) & ~(shaderGroupHandleAlignment - 1);
		VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{
			.deviceAddress = std::static_pointer_cast<BufferVulkan>(buffer)->GetDeviceAddress(),
			.stride = handleSizeAligned
		};
		stridedDeviceAddressRegionKHR.size = count * handleSizeAligned;

		region = stridedDeviceAddressRegionKHR;
	}
}
