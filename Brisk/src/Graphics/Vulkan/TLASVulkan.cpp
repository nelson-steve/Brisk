#include "pch.hpp"
#include "TLASVulkan.hpp"
#include "BufferVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
#include "CommandBufferVulkan.hpp"
#include "BLASVulkan.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Brisk
{
	void TLASVulkan::Build(std::shared_ptr<BLAS> blas) {
		auto draws = SceneManager::pActiveScene->GetDrawsOpaque();
		uint32_t drawCount = draws.size();

		std::shared_ptr<CommandBufferVulkan> cmd = std::make_shared<CommandBufferVulkan>();
		cmd->Allocate(CommandBuffer::PoolType::Graphics);

		std::shared_ptr<BufferVulkan> instancesBuffer = std::make_shared<BufferVulkan>();
		BufferDesc instancesBufferDesc{};
		instancesBufferDesc.p_Size = sizeof(VkAccelerationStructureInstanceKHR) * draws.size();
		instancesBufferDesc.p_Usage = Core::BufferUsage::AccelerationStructureBuildInputReadOnly | Core::BufferUsage::ShaderDeviceAddress;
		instancesBufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
		instancesBufferDesc.p_GpuMapped = true;
		instancesBufferDesc.p_Persistant = true;
		instancesBuffer->Init(instancesBufferDesc);

		std::vector<VkAccelerationStructureInstanceKHR> instances(drawCount);
		for (uint32_t i = 0; i < drawCount; i++) {
			VkAccelerationStructureInstanceKHR& instance = instances[i];

			MeshTransform t = SceneManager::pActiveScene->GetTransforms()[SceneManager::pActiveScene->GetDrawsOpaque()[i].transformIndex];
			glm::quat q = glm::quat(
				t.orientation.w,
				t.orientation.x,
				t.orientation.y,
				t.orientation.z
			);

			glm::mat3 xform = glm::transpose(glm::mat3_cast(q)) * t.scale;

			memcpy(instance.transform.matrix[0], &xform[0], sizeof(float) * 3);
			memcpy(instance.transform.matrix[1], &xform[1], sizeof(float) * 3);
			memcpy(instance.transform.matrix[2], &xform[2], sizeof(float) * 3);
			instance.transform.matrix[0][3] = t.position.x;
			instance.transform.matrix[1][3] = t.position.y;
			instance.transform.matrix[2][3] = t.position.z;
			instance.mask = 0xFF;

			instance.instanceCustomIndex = draws[i].meshIndex;
			instance.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;
			instance.accelerationStructureReference = std::static_pointer_cast<BLASVulkan>(blas)->blasAddresses[draws[i].meshIndex];
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.mask = 0xFF;
			void* ptr = instancesBuffer->GetMapped();
			memcpy(static_cast<VkAccelerationStructureInstanceKHR*>(ptr) + i, &instance, sizeof(VkAccelerationStructureInstanceKHR));
		}

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
		vkGetAccelerationStructureBuildSizesKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &drawCount, &sizeInfo);

		std::shared_ptr<BufferVulkan> scratchBuffer = std::make_shared<BufferVulkan>();
		{
			VmaAllocator cachedAllocator = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetVmaAllocator();

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = sizeInfo.accelerationStructureSize;
			bufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			if (vmaCreateBuffer(cachedAllocator, &bufferInfo, &allocInfo, &buffer, &m_Allocation, nullptr) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create buffer");
			}

			// Creating scratch buffer
			BufferDesc bufferDesc{};
			bufferDesc.p_Size = std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
			bufferDesc.p_Usage = Core::BufferUsage::StorageBuffer | Core::BufferUsage::ShaderDeviceAddress | Core::BufferUsage::AccelerationStructureStorage;
			bufferDesc.p_Memory = BufferDesc::MemoryUsage::GPU_Only;
			bufferDesc.p_AllowSRV = true;
			bufferDesc.p_Aligned = true;
			scratchBuffer->Init(bufferDesc);
		}

		VkAccelerationStructureCreateInfoKHR accelerationInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		accelerationInfo.buffer = buffer;
		accelerationInfo.size = sizeInfo.accelerationStructureSize;
		accelerationInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		if (vkCreateAccelerationStructureKHR(std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter())->GetDevice(), &accelerationInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create acceleration structure");
		}

		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		geometry.geometry.instances.data.deviceAddress = instancesBuffer->GetDeviceAddress();

		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		buildInfo.dstAccelerationStructure = handle;
		buildInfo.scratchData.deviceAddress = scratchBuffer->GetDeviceAddress();

		VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
		buildRange.primitiveCount = drawCount;
		const VkAccelerationStructureBuildRangeInfoKHR* buildRangePtr = &buildRange;

		cmd->Bind();
		vkCmdBuildAccelerationStructuresKHR(cmd->Get(), 1, &buildInfo, &buildRangePtr);

		cmd->UnBind();

		GpuAdapter::SubmitInfo submitInfo{};
		submitInfo.pCmdBuffers = { cmd };
		Application::GetGpuAdapter()->SubmitGraphics(submitInfo, nullptr);
		Application::GetGpuAdapter()->WaitIdle();
	}
}