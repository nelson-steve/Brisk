#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.hpp"
#include "Pipeline.hpp"

namespace Brisk
{
	class SBT {
	public:
		virtual void Init(std::shared_ptr<Pipeline> pipeline) = 0;

		static std::shared_ptr<SBT> Create();
	protected:
		std::shared_ptr<Buffer> buffer;
	};
}