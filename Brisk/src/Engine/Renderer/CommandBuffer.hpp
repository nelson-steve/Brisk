#pragma once
// INCLUDES
#include "Core/Core.hpp"
//-------------------
#include <memory>
#include <functional>
//-------------------

namespace Brisk {
	class CommandBuffer {
		DEFINE_BASE_CLASS_CONSTRUCTOR(CommandBuffer)
	public:
		virtual void Bind() = 0;
		void RecordCommand(std::function<void()> command){
			m_Commands.push_back(command);
		}
		virtual void UnBind() = 0;
		virtual void Reset() = 0;

		static std::shared_ptr<CommandBuffer> Create();
	protected:
		std::vector<std::function<void()>> m_Commands;
	};
}