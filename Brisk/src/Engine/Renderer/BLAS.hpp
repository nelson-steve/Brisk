#pragma once

#include "Buffer.hpp"

namespace Brisk
{
	class BLAS {
	public:
		virtual void Build(std::shared_ptr<Buffer> vb, std::shared_ptr<Buffer> ib) = 0;
	};
}