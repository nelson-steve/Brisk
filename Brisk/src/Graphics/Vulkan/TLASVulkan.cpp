#include "TLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk
{
	void TLASVulkan::Init(const TLASSpecs& specs, std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<BLAS> blas) {
		VkTransformMatrixKHR transformMatrix = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f };

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		//instance.accelerationStructureReference = blas->GetDeviceAddress();

 		std::shared_ptr<BufferVulkan> instancesBuffer = std::make_shared<BufferVulkan>();
		BufferDesc instancesBufferDesc{};
		instancesBufferDesc.p_Size = sizeof(VkAccelerationStructureInstanceKHR);
		instancesBufferDesc.p_Usage = Core::BufferUsage::AccelerationStructureBuildInputReadOnly | Core::BufferUsage::ShaderDeviceAddress;
		instancesBufferDesc.p_Memory = BufferDesc::MemoryUsage::CPU_To_GPU; // host visibel host coherent
		instancesBuffer->Init(instancesBufferDesc);

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = instancesBuffer->GetDeviceAddress();

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
		accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		uint32_t primitiveCount = specs.primitiveCount;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(
			std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&primitiveCount,
			&accelerationStructureBuildSizesInfo);

		{
			VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetVmaAllocator();

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
			bufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			if (vmaCreateBuffer(cachedAllocator, &bufferInfo, &allocInfo, &buffer, &m_Allocation, nullptr) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create buffer");
			}

			VkAccelerationStructureCreateInfoKHR accelerationStructureCreatInfo{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.buffer = buffer,
				.size = accelerationStructureBuildSizesInfo.accelerationStructureSize,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
			};
			vkCreateAccelerationStructureKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationStructureCreatInfo, nullptr, &handle);
			VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.accelerationStructure = handle
			};
			deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationDeviceAddressInfo);
		}

		std::shared_ptr<BufferVulkan> scratchBuffer = std::make_shared<BufferVulkan>();
		BufferDesc bufferDesc{};
		bufferDesc.p_Size = accelerationStructureBuildSizesInfo.buildScratchSize + 256;
		bufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::ShaderDeviceAddress;
		bufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		bufferDesc.p_AllowSRV = true;
		scratchBuffer->Init(bufferDesc);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		const uint64_t alignment = 256;
		uint64_t alignedScratchAddress =
			(scratchBuffer->GetDeviceAddress() + alignment - 1) & ~(alignment - 1);
		accelerationBuildGeometryInfo.scratchData.deviceAddress = alignedScratchAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = 1;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		vkCmdBuildAccelerationStructuresKHR(
			std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(),
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());

		scratchBuffer->Release();
	}
}