#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk 
{
	class GraphicsPipeline {
	public:
		void Create();
		void Release();
	private:
		std::vector<char>& ReadShaderFile(const std::string& fileName);
		VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode);
	private:
		VkShaderModule vertShader;
		VkShaderModule fragShader;
	};
}