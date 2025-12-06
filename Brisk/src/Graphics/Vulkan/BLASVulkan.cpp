#include "pch.hpp"
#include "BLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"

namespace Brisk
{
	const VkBuildAccelerationStructureFlagsKHR kBuildBLAS = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

	void BLASVulkan::Build(std::shared_ptr<Buffer> vb, std::shared_ptr<Buffer> ib) {
		VkDeviceAddress vbAddress{};
		VkDeviceAddress ibAddress{};

		const uint32_t meshCount = SceneManager::pActiveScene->GetMeshes().size();

		vbAddress = std::static_pointer_cast<BufferVulkan>(vb)->GetDeviceAddress();
		ibAddress = std::static_pointer_cast<BufferVulkan>(ib)->GetDeviceAddress();

		std::vector<uint32_t> primitiveCounts(meshCount);
		std::vector<VkAccelerationStructureGeometryKHR> geometries(meshCount);
		std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildInfos(meshCount);

		std::vector<size_t> accelerationOffsets(meshCount);
		std::vector<size_t> accelerationSizes(meshCount);
		std::vector<size_t> scratchSizes(meshCount);

		size_t totalAccelerationSize = 0;
		size_t totalPrimitiveCount = 0;
		size_t maxScratchSize = 0;

		const size_t kAlignment = 256;
		const size_t kDefaultScratch = 32 * 1024 * 1024;

		for (int i = 0; i < meshCount; i++) {
			const Mesh& mesh = SceneManager::pActiveScene->GetMeshes()[i];

			primitiveCounts[i] = mesh.indexCount / 3;

			uint32_t vertexOffset = mesh.vertexOffset;
			uint32_t indexOffset = mesh.indexOffset;
			uint32_t maxVertex = mesh.vertexCount - 1;

			VkAccelerationStructureGeometryKHR accelerationStructureGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
			accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = vbAddress + vertexOffset * sizeof(Vertex);
			accelerationStructureGeometry.geometry.triangles.maxVertex = maxVertex;
			accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
			accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = ibAddress + indexOffset * sizeof(uint32_t);
			accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
			accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
			geometries[i] = accelerationStructureGeometry;

			VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
			accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			accelerationStructureBuildGeometryInfo.flags = kBuildBLAS /*| VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR*/;
			accelerationStructureBuildGeometryInfo.geometryCount = 1;
			accelerationStructureBuildGeometryInfo.pGeometries = &geometries[i];
			accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR	;

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

			totalAccelerationSize = (totalAccelerationSize + accelerationStructureBuildSizesInfo.accelerationStructureSize + kAlignment - 1) & ~(kAlignment - 1);
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
			bufferDesc.p_Size = std::max(kDefaultScratch, maxScratchSize);
			bufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::ShaderDeviceAddress;
			bufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
			scratchBuffer->Init(bufferDesc);
		}

		std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges(meshCount);
		std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> buildRangePtrs(meshCount);

		// Creating
		blases.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			VkAccelerationStructureCreateInfoKHR accelerationStructureCreatInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			accelerationStructureCreatInfo.buffer = buffer;
			accelerationStructureCreatInfo.offset = accelerationOffsets[i];
			accelerationStructureCreatInfo.size = accelerationSizes[i];
			accelerationStructureCreatInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			if (vkCreateAccelerationStructureKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationStructureCreatInfo, nullptr, &blases[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create acceleration structure");
			}
		}

		std::shared_ptr<CommandBufferVulkan> cmd = std::make_shared<CommandBufferVulkan>();
		cmd->Allocate(CommandBuffer::PoolType::Graphics);

		// Building
		for (int i = 0; i < meshCount; i++) {
			cmd->Reset();
			cmd->Bind();

			buildInfos[i].scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();
			buildInfos[i].dstAccelerationStructure = blases[i];
			buildRanges[i].primitiveCount = primitiveCounts[i];
			buildRangePtrs[i] = &buildRanges[i];

			vkCmdBuildAccelerationStructuresKHR(
				cmd->Get(),
				1,
				&buildInfos[i],
				&buildRangePtrs[i]);

			cmd->UnBind();

			Queue::SubmitInfo submitInfo{};
			submitInfo.pCmdBuffers.push_back(cmd);
			Application::GetRenderer()->m_GraphicsQueue0->Submit(submitInfo, nullptr);
			Application::GetGpuAdapter()->WaitIdle();
		}

		blasAddresses.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {

			VkAccelerationStructureDeviceAddressInfoKHR info = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
			info.accelerationStructure = blases[i];

			blasAddresses[i] = vkGetAccelerationStructureDeviceAddressKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &info);
		}

		scratchBuffer->Release();
	}
}