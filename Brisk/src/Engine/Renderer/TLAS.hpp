#pragma once

#include "Buffer.hpp"
#include "BLAS.hpp"

#include <memory>

namespace Brisk
{
	struct TLASSpecs {
		uint32_t primitiveCount;
	};
	class TLAS {
	public:
		virtual void Init(const TLASSpecs& specs, std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<BLAS> blas) = 0;
	};

}