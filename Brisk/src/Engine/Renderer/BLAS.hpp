#pragma once

#include "Buffer.hpp"
#include "../Scene.hpp"

#include <memory>

namespace Brisk
{
	class BLAS {
	public:
		virtual void Build(const std::vector<Mesh> meshes, std::shared_ptr<Buffer> vb, std::shared_ptr<Buffer> ib) = 0;
	};
}