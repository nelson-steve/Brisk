#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.hpp"

namespace Brisk
{
	class SBT {
	public:
		enum Type {
			Raygen,
			Miss,
			Hit
		};
		virtual void Init(Type type, uint32_t count) = 0;

		static std::shared_ptr<SBT> Create();
	protected:
		std::shared_ptr<Buffer> buffer;
	};
}