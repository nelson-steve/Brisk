#pragma once

#include "Graphics/GPUDevice.hpp"
#include "Graphics/Vulkan/GraphicsPipelineVulkan.hpp"

namespace Brisk {
	class Renderer {
	public:
		void Initialize();
		void Release();

		void PreProcess();
		void Render();
		void PostProcess();
	private:
		GraphicsPipelineVulkan* m_DefaultGraphicsPipeline;
	};
}