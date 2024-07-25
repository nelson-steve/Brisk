#pragma once

#include"Graphics/GPUDevice.hpp"

namespace Brisk {
	class GraphicsDeviceDirectX11 : GPUDevice {
	public:
		void Create() override;
		void SetupGraphicsPipeline(std::vector<ShaderInfo> shaders) override;
		void ReleaseGraphicsPipeline() override;
		void Draw() override;
		void WaitDeviceIdle() override;
		void Release() override;
	private:
	};
}