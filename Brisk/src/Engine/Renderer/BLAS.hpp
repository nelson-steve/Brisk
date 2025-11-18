#pragma once

#include "Buffer.hpp"

#include <memory>

namespace Brisk
{
	struct BLASSpecs {
		std::shared_ptr<Buffer> vertexBuffer;
		std::shared_ptr<Buffer> indexBuffer;
		uint32_t noOfTriangles;
		uint32_t noOfVertices;
		uint32_t vertexStride;
	};
	class BLAS {
	public:
		virtual void Init(const BLASSpecs& specs, std::shared_ptr<CommandBuffer> cmd) = 0;
	};

}