#include "BLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

#include <algorithm>

namespace Brisk
{
	void BLASVulkan::Build(const std::vector<Mesh> meshes, std::shared_ptr<Buffer> vb, std::shared_ptr<Buffer> ib) {
		VkDeviceAddress vbAddress{};
		VkDeviceAddress ibAddress{};

		vbAddress = std::static_pointer_cast<BufferVulkan>(vb)->GetDeviceAddress();
		ibAddress = std::static_pointer_cast<BufferVulkan>(ib)->GetDeviceAddress();

		std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildInfos(meshes.size());

		std::vector<size_t> accelerationOffsets(meshes.size());
		std::vector<size_t> accelerationSizes(meshes.size());
		std::vector<size_t> scratchSizes(meshes.size());
		uint32_t totalAccelerationSize = 0;
		uint32_t totalPrimitiveCount = 0;
		size_t maxScratchSize = 0;

		// Querying
		std::vector<uint32_t> primitiveCounts(meshes.size());
		for (int i = 0; i < meshes.size(); i++) {
			primitiveCounts[i] = meshes[i].indexCount / 3;

			VkAccelerationStructureGeometryKHR accelerationStructureGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
			accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = vbAddress + meshes[i].vertexOffset * sizeof(Vertex);
			accelerationStructureGeometry.geometry.triangles.maxVertex = meshes[i].vertexCount;
			accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
			accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = ibAddress + meshes[i].indexOffset * sizeof(uint32_t);
			accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
			accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

			VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
			accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			accelerationStructureBuildGeometryInfo.geometryCount = 1;
			accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

			buildInfos[i] = accelerationStructureBuildGeometryInfo;

			VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };

			vkGetAccelerationStructureBuildSizesKHR(
				std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(),
				VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&accelerationStructureBuildGeometryInfo,
				&primitiveCounts[i],
				&accelerationStructureBuildSizesInfo);

			accelerationOffsets[i] = totalAccelerationSize;
			accelerationSizes[i] = accelerationStructureBuildSizesInfo.accelerationStructureSize;
			scratchSizes[i] = accelerationStructureBuildSizesInfo.buildScratchSize;

			uint32_t alignment = 256;
			totalAccelerationSize = (totalAccelerationSize + accelerationStructureBuildSizesInfo.accelerationStructureSize + alignment - 1) & ~(alignment - 1);
			totalPrimitiveCount += primitiveCounts[i];
			maxScratchSize = std::max(maxScratchSize, size_t(accelerationStructureBuildSizesInfo.buildScratchSize));
		}

		// Creating buffers
		std::shared_ptr<BufferVulkan> scratchBuffer = std::make_shared<BufferVulkan>();
		{
			VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetVmaAllocator();

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = totalAccelerationSize;
			bufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			if (vmaCreateBuffer(cachedAllocator, &bufferInfo, &allocInfo, &buffer, &m_Allocation, nullptr) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create buffer");
			}

			// Creating scratch buffer
			BufferDesc bufferDesc{};
			bufferDesc.p_Size = maxScratchSize;
			bufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::ShaderDeviceAddress;
			bufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
			bufferDesc.p_AllowSRV = true;
			scratchBuffer->Init(bufferDesc);
		}

		std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges(meshes.size());
		std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> buildRangePtrs(meshes.size());

		// Creating
		blases.resize(meshes.size());
		for (int i = 0; i < meshes.size(); i++) {
			VkAccelerationStructureCreateInfoKHR accelerationStructureCreatInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			accelerationStructureCreatInfo.buffer = buffer;
			accelerationStructureCreatInfo.offset = accelerationOffsets[i];
			accelerationStructureCreatInfo.size = accelerationSizes[i];
			accelerationStructureCreatInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			vkCreateAccelerationStructureKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationStructureCreatInfo, nullptr, &blases[i]);
			VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.accelerationStructure = blases[i]
			};
		}

		std::shared_ptr<CommandBufferVulkan> cmd = std::make_shared<CommandBufferVulkan>();
		cmd->Allocate(CommandBuffer::PoolType::Graphics);
		cmd->Bind();

		// Building
		for (int i = 0; i < meshes.size(); i++) {
			buildInfos[i].scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();
			buildInfos[i].dstAccelerationStructure = blases[i];
			buildRanges[i].primitiveCount = primitiveCounts[i];
			buildRangePtrs[i] = &buildRanges[i];

			vkCmdBuildAccelerationStructuresKHR(
				cmd->Get(),
				1,
				&buildInfos[i],
				&buildRangePtrs[i]);

			VkMemoryBarrier2 memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
			memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			memoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
			memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
			memoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

			VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			dependencyInfo.memoryBarrierCount = 1;
			dependencyInfo.pMemoryBarriers = &memoryBarrier;

			vkCmdPipelineBarrier2(cmd->Get(), &dependencyInfo);
		}

		for (int i = 0; i < meshes.size(); i++) {

			VkAccelerationStructureDeviceAddressInfoKHR info = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
			info.accelerationStructure = blases[i];

			blasAddresses[i] = vkGetAccelerationStructureDeviceAddressKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &info);
		}

		cmd->UnBind();
		Queue::SubmitInfo submitInfo{};
		submitInfo.pCmdBuffers.push_back(cmd);
		Application::GetRenderer()->m_GraphicsQueue0->Submit(submitInfo, nullptr);
		Application::GetGpuAdapter()->WaitIdle();

		scratchBuffer->Release();
	}
}