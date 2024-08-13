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
		m_Renderpass->BeginRenderPass(0);
		m_Renderpass->BindPipeline(m_DefaultGraphicsPipeline->GetPipeline());
		m_Renderpass->EndRenderPass();
		
	}

	void Renderer::PostProcess() {

	}
}