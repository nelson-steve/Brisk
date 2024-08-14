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

		m_RenderPass = RenderPass::Create();

		m_DefaultGraphicsPipeline = new GraphicsPipelineVulkan();
		m_DefaultGraphicsPipeline->Create(modules);
	}

	void Renderer::Release() {

	}

	void Renderer::PreProcess() {

	}

	void Renderer::Render() {
		m_RenderPass->BeginRenderPass();
		m_RenderPass->BindPipeline(m_DefaultGraphicsPipeline);

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->PrepreFrame(
			static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer());
		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Draw(
			static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer());

		m_RenderPass->EndRenderPass();

		static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Submit(
			static_cast<RenderPassVulkan*>(m_RenderPass));
	}

	void Renderer::PostProcess() {

	}
}