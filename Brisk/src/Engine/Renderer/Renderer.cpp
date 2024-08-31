#include "Renderer.hpp"
#include "Engine/Engine.hpp"

namespace Brisk {


	//void Renderer::Initialize() {
	//	// Create default graphics pipeline
	//	ShaderInfo vertexShader;
	//	vertexShader.Path = "Shaders/Vulkan/Compiled/TriangleVS.spv";
	//	vertexShader.Type = ShaderType::Vertex;

	//	ShaderInfo fragmentShader;
	//	fragmentShader.Path = "Shaders/Vulkan/Compiled/TriangleFS.spv";
	//	fragmentShader.Type = ShaderType::Fragment;

	//	std::vector<ShaderInfo> shaders{
	//		vertexShader,
	//		fragmentShader,
	//	};
	//	std::vector<ShaderModule> modules;
	//	for (ShaderInfo shader : shaders) {
	//		modules.push_back(ShaderManager::CreateShaderModule(shader));
	//	}

	//	m_RenderPass = RenderPassFactory::CreateRenderPass();
	//	m_RenderPass->Create();

	//	m_DefaultGraphicsPipeline = new GraphicsPipelineVulkan();
	//	m_DefaultGraphicsPipeline->Create(modules);

	//	//std::vector<Point> vertices = {
	//	//	{{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	//	//	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	//	//	{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	//	//};
	//	std::vector<Point> vertices = {
	//		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	//		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	//		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},

	//		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	//		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	//		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	//	};

	//	m_VertexBuffer = new BufferVulkan();
	//	m_VertexBuffer->Create(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	//	m_VertexBuffer->Allocate();
	//	m_VertexBuffer->MapMemory(vertices);
	//}

	//void Renderer::Release() {
	//	m_VertexBuffer->Release();
	//	m_DefaultGraphicsPipeline->Release();
	//	m_RenderPass->Release();
	//}

	//void Renderer::PreProcess() {

	//}

	//void Renderer::Render() {
	//	if (Engine::s_GPUContext->Sync())
	//		return;

	//	m_RenderPass->BeginRenderPass();
	//	m_RenderPass->BindPipeline(m_DefaultGraphicsPipeline);

	//	static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->PrepreFrame(
	//		static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer());
	//	static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Draw(
	//		static_cast<RenderPassVulkan*>(m_RenderPass)->GetCommandBuffer(), *m_VertexBuffer);

	//	m_RenderPass->EndRenderPass();

	//	static_cast<GraphicsDeviceVulkan*>(Engine::s_GPUContext)->Submit(
	//		static_cast<RenderPassVulkan*>(m_RenderPass));
	//}

	//void Renderer::PostProcess() {

	//}
}