#include "BLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk
{
	void BLASVulkan::Init(const BLASSpecs& specs, std::shared_ptr<CommandBuffer> cmd) {
		VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
		VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

		vertexBufferDeviceAddress.deviceAddress = std::static_pointer_cast<BufferVulkan>(specs.vertexBuffer)->GetDeviceAddress();
		indexBufferDeviceAddress.deviceAddress = std::static_pointer_cast<BufferVulkan>(specs.indexBuffer)->GetDeviceAddress();

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.maxVertex = specs.noOfVertices - 1;
		accelerationStructureGeometry.geometry.triangles.vertexStride = specs.vertexStride;
		accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
		accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
		accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(
			std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&specs.noOfTriangles,
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
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
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
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
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
		accelerationStructureBuildRangeInfo.primitiveCount = specs.noOfTriangles;
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