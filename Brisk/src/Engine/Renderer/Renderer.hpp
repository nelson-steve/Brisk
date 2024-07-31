#pragma once

#include "Graphics/GPUDevice.hpp"

namespace Brisk {
	class Renderer {
	public:
		void Create(GPUDevice* device);
		void Release();

		void PreProcess();
		void Render();
		void PostProcess();
	private:
		GPUDevice* m_GPUDevice;
	};
}