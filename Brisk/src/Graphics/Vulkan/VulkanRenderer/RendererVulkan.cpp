#include "RendererVulkan.hpp"

namespace Brisk {
	void RendererVulkan::Create() {
		m_GpuContext = new GpuContextVulkan();
		m_GpuContext->Create();
	
		std::vector<GpuDeviceVulkan::QueueType> queueTypes;
		std::vector<GpuDeviceVulkan::Feature> features;
		GpuDeviceVulkan::GpuRequirements req;
		m_GpuContext->CreateDevice(req);
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