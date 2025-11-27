#include "TLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "BLASVulkan.hpp"

namespace Brisk
{
	void TLASVulkan::Build(std::shared_ptr<BLAS> blas,std::vector<MeshDraw> draws) {
		// Create instance buffer
		std::shared_ptr<BufferVulkan> instancesBuffer = std::make_shared<BufferVulkan>();
		BufferDesc instancesBufferDesc{};
		instancesBufferDesc.p_Size = sizeof(VkAccelerationStructureInstanceKHR) * draws.size();
		instancesBufferDesc.p_Usage = Core::BufferUsage::AccelerationStructureBuildInputReadOnly | Core::BufferUsage::ShaderDeviceAddress;
		instancesBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		instancesBuffer->Init(instancesBufferDesc);


		// Fill instance buffer
		for (size_t i = 0; i < draws.size(); ++i)
		{
			const MeshDraw& draw = draws[i];
			assert(draw.meshIndex < std::static_pointer_cast<BLASVulkan>(blas)->blases.size());

			//VkAccelerationStructureInstanceKHR instance{};
			//instance.transform = transformMatrix;
			//instance.instanceCustomIndex = i;
			//instance.mask = 0xFF;
			//instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			//instance.accelerationStructureReference = std::static_pointer_cast<BLASVulkan>(blas)->blasAddresses[i];

			//memcpy(static_cast<VkAccelerationStructureInstanceKHR*>(tlasInstanceBuffer.data) + i, &instance, sizeof(VkAccelerationStructureInstanceKHR));
		}

		// Create acceleration structure
		VkAccelerationStructureGeometryKHR geometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry.geometry.instances.data.deviceAddress = instancesBuffer->GetDeviceAddress();

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		//vkGetAccelerationStructureBuildSizesKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &primitiveCount, &sizeInfo);

		printf("TLAS accelerationStructureSize: %.2f MB, scratchSize: %.2f MB, updateScratch: %.2f MB\n", double(sizeInfo.accelerationStructureSize) / 1e6, double(sizeInfo.buildScratchSize) / 1e6, double(sizeInfo.updateScratchSize) / 1e6);

		std::shared_ptr<BufferVulkan> tlasBuffer = std::make_shared<BufferVulkan>();
		BufferDesc tlasBufferDesc{};
		tlasBufferDesc.p_Size = sizeInfo.accelerationStructureSize;
		tlasBufferDesc.p_Usage = Core::BufferUsage::AccelerationStructureStorage;
		tlasBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		instancesBuffer->Init(tlasBufferDesc);

		std::shared_ptr<BufferVulkan> scratchBuffer = std::make_shared<BufferVulkan>();
		BufferDesc scratchBufferDesc{};
		scratchBufferDesc.p_Size = std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
		scratchBufferDesc.p_Usage = Core::BufferUsage::AccelerationStructureStorage | Core::BufferUsage::ShaderDeviceAddress;
		scratchBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		instancesBuffer->Init(scratchBufferDesc);

		VkAccelerationStructureCreateInfoKHR accelerationInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		accelerationInfo.buffer = tlasBuffer->Get();
		accelerationInfo.size = sizeInfo.accelerationStructureSize;
		accelerationInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		VkAccelerationStructureKHR tlas = nullptr;
		if (vkCreateAccelerationStructureKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationInfo, nullptr, &tlas) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create acceleration structure");
		}


		// Build acceleration structure

		std::shared_ptr<CommandBufferVulkan> cmd = std::make_shared<CommandBufferVulkan>();
		cmd->Allocate(CommandBuffer::PoolType::Graphics);
		cmd->Bind();

		//VkAccelerationStructureGeometryKHR geometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		//geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		//geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		//geometry.geometry.instances.data.deviceAddress = instancesBuffer->GetDeviceAddress();

		//VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		//buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		//buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		////buildInfo.mode = mode;
		//buildInfo.geometryCount = 1;
		//buildInfo.pGeometries = &geometry;

		//buildInfo.srcAccelerationStructure = tlas;
		//buildInfo.dstAccelerationStructure = tlas;
		//buildInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

		VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
		//buildRange.primitiveCount = primitiveCount;
		const VkAccelerationStructureBuildRangeInfoKHR* buildRangePtr = &buildRange;

		//vkCmdBuildAccelerationStructuresKHR(cmd->Get(), 1, &buildInfo, &buildRangePtr);

		cmd->UnBind();
		Queue::SubmitInfo submitInfo{};
		submitInfo.pCmdBuffers.push_back(cmd);
		Application::GetRenderer()->m_GraphicsQueue0->Submit(submitInfo, nullptr);
		Application::GetGpuAdapter()->WaitIdle();

		VkMemoryBarrier2 memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
		memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		memoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;

		VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.memoryBarrierCount = 1;
		dependencyInfo.pMemoryBarriers = &memoryBarrier;

		vkCmdPipelineBarrier2(cmd->Get(), &dependencyInfo);
	}
}