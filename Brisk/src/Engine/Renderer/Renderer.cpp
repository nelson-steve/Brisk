#include "Renderer.hpp"
#include "Engine/Engine.hpp"
#include <Graphics/ShaderManager.hpp>

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

		m_DefaultGraphicsPipeline = new GraphicsPipelineVulkan();
		m_DefaultGraphicsPipeline->Create(modules);

		Engine::s_GPUContext->SetupGraphicsPipeline(
			{
				vertexShader,
				fragmentShader,
			}
		);
	}
}