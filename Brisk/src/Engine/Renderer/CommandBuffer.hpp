#pragma once
// INCLUDES
#include "Core/Core.hpp"
//-------------------

namespace Brisk {
	class CommandBuffer {
	public:
		enum class PoolType {
			Graphics,
			Compute,
			Transfer
		};
		DEFINE_BASE_CLASS_CONSTRUCTOR(CommandBuffer)
	public:
		virtual void Bind(bool singleUse = false) = 0;

		virtual void Allocate(PoolType type) = 0;
		virtual void UnBind() = 0;
		virtual void Reset() = 0;

		static std::shared_ptr<CommandBuffer> Create();
	};
}