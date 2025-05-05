#pragma once

// INCLUDES
#include "Core/Core.hpp"
//---------------
#include <memory>
//----------------

namespace Brisk
{
	class Shader
	{
		DEFINE_BASE_CLASS_CONSTRUCTOR(Shader)
	public:
		enum ShaderType
		{
			VERTEX,
			FRAGMENT,
		};

		static std::shared_ptr<Shader> Create();
	public:
	};
}