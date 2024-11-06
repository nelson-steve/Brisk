#pragma once
#include <memory>

namespace Brisk {
	class CommandBuffer {
	public:
		virtual void Bind() = 0;
		void RecordCommand(std::function<void()> command){
			m_Commands.push_back(command);
		}
		virtual void UnBind() = 0;

		static std::shared_ptr<CommandBuffer> Create();
	protected:
		std::vector<std::function<void()>> m_Commands;
	};
}