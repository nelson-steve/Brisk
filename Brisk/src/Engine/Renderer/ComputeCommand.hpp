#pragma once

// INCLUDES
#include "RendererAPI.hpp"
#include "Buffer.hpp"
//------------------

namespace Brisk
{
	class ComputeCommand {
	public:
		inline static void CmdDispatch(std::shared_ptr<CommandBuffer> cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
			s_ComputeAPI->CmdDispatch(cmd, groupCountX, groupCountY, groupCountZ);
		}
	public:
		static ComputeAPI* s_ComputeAPI;
	};
}