#pragma once

#include "Texture.hpp"

#include <memory>
#include <string>

namespace Brisk 
{
	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(std::vector<std::shared_ptr<Texture>> inputs, std::vector<std::shared_ptr<Texture>> outputs) = 0;
		virtual void Bind() = 0;
	};
};
