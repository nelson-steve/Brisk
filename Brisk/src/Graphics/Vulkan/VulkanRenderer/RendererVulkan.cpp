#include "RendererVulkan.hpp"

namespace Brisk {
	void RendererVulkan::Create() {
		m_GPUContext = new GPUContextVulkan();
		m_GPUContext->Create();
	}

	void RendererVulkan::Release() {

	}

	void RendererVulkan::PreRender() {

	}

	void RendererVulkan::Render() {

	}

	void RendererVulkan::PostRender() {

	}
}