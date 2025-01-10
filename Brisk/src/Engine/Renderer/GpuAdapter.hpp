#pragma once
// INCLUDES
#include <memory>
//---------------

namespace Brisk
{
	class GpuAdapter {
	public:
		virtual void Init() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}