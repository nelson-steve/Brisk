#pragma once

#include "Engine/Renderer/RendererAPI.hpp"

#include <Volk/volk.h>
#include "CommandBufferVulkan.hpp"

namespace Brisk
{
	class ComputeAPIVulkan : public ComputeAPI {
		virtual void CmdDispatch(std::shared_ptr<CommandBuffer> cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
			vkCmdDispatch(std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get(), groupCountX, groupCountY, groupCountZ);
		}
	};
}