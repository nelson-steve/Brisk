#pragma once
// INCLUDES
#include "Core/Core.hpp"
//---------------
#include <memory>
//---------------

namespace Brisk
{
	class GpuAdapter {
		DEFINE_BASE_CLASS_CONSTRUCTOR(GpuAdapter)
	public:
		virtual void Init() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}