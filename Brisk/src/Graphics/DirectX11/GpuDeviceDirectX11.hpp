#pragma once

#include <d3d12.h>

namespace Brisk 
{
	class GpuDeviceDirectX11 {
	public:
		void Create();
		void WaitDeviceIdle();
		void Release();
	private:
	};
}