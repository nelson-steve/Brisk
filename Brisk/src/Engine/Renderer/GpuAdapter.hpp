#pragma once
#include <memory>

namespace Brisk
{
	class GpuAdapter {
	public:
		void Init();

		static std::shared_ptr<GpuAdapter> Create();
	};
}