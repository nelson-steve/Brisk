#include "Renderer.hpp"
#include "Engine/Engine.hpp"
#include <Graphics/ShaderManager.hpp>
#include <Graphics/Vulkan/SwapchainVulkan.hpp>

namespace Brisk {
	void Renderer::Initialize() {
		// Create default graphics pipeline
		ShaderInfo vertexShader;
		vertexShader.Path = "Shaders/Vulkan/Compiled/TriangleVS.spv";
		vertexShader.Type = ShaderType::Vertex;

		ShaderInfo fragmentShader;
		fragmentShader.Path = "Shaders/Vulkan/Compiled/TriangleFS.spv";
		fragmentShader.Type = ShaderType::Fragment;

		std::vector<ShaderInfo> shaders{
			vertexShader,
			fragmentShader,
		};
		std::vector<ShaderModule> modules;
		for (ShaderInfo shader : shaders) {
			modules.push_back(ShaderManager::CreateShaderModule(shader));
		}

		m_Renderpass = new RenderpassVulkan();
		m_Renderpass->Create(static_cast<SwapchainVulkan*>(Engine::s_Swapchain)->GetSwapchainImages().size());

		m_DefaultGraphicsPipeline = new GraphicsPipelineVulkan();
		m_DefaultGraphicsPipeline->Create(modules);
	}

	void Renderer::Release() {

	}

	void Renderer::PreProcess() {

	}

	void Renderer::Render() {
		m_Renderpass->Reset();
		m_Renderpass->BeginRenderPass(static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->GetImageIndex());
		m_Renderpass->BindPipeline(m_DefaultGraphicsPipeline->GetPipeline());

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->PrepreFrame(m_Renderpass->GetCommandBuffer());
		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Draw(m_Renderpass->GetCommandBuffer());

		m_Renderpass->EndRenderPass();

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Submit(m_Renderpass);
	}

	void Renderer::PostProcess() {

	}
}