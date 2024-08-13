#pragma once

#include"Graphics/GPUDevice.hpp"

namespace Brisk {
	class GraphicsDeviceDirectX11 : GPUContext{
	public:
		void Create() override;
		void WaitDeviceIdle() override;
		void Release() override;
	private:
	};
}