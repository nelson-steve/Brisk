#include "RendererVulkan.hpp"
#include "Graphics/Vulkan/SwapchainVulkan.hpp"

namespace Brisk {
	void RendererVulkan::Create() {
		m_GpuContext = new GpuContextVulkan();
		m_GpuContext->Create();
	
		std::vector<GpuDeviceVulkan::QueueType> queueTypes;
		std::vector<GpuDeviceVulkan::DeviceFeatures> features;
		GpuDeviceVulkan::GpuRequirements req;
		m_GpuContext->CreateDevice(req);
	}

	void RendererVulkan::SetupRenderTargets(const Swapchain* swapchain) {
		const SwapchainVulkan* swpchain = static_cast<const SwapchainVulkan*>(swapchain);
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