#pragma once
#include <memory>

namespace Brisk {
	class CommandBuffer {
	public:
		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		static std::shared_ptr<CommandBuffer> Create();
	};
}