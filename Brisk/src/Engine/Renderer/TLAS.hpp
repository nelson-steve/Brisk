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
		virtual void Build(std::shared_ptr<BLAS> blas, std::vector<MeshDraw> draws) = 0;
	};

}