#include "GraphicsPipeline.hpp"
#include "Core/Log.hpp"
#include "Engine/Engine.hpp"

#include <fstream>

namespace Brisk {
	void GraphicsPipeline::Create() {
        auto vertShaderCode = ReadShaderFile("Shaders/Vulkan/Compiled/TriangleVS.spv");
        auto fragShaderCode = ReadShaderFile("Shaders/Vulkan/Compiled/TriangleFS.spv");

        vertShader = CreateShaderModule(vertShaderCode);
        fragShader = CreateShaderModule(fragShaderCode);
	}

    void GraphicsPipeline::Release() {
        vkDestroyShaderModule(Engine::s_PhysicalDevice->GetDevice(), vertShader, nullptr);
        vkDestroyShaderModule(Engine::s_PhysicalDevice->GetDevice(), fragShader, nullptr);
    }

	std::vector<char>& GraphicsPipeline::ReadShaderFile(const std::string & fileName) {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            BRISK_CORE_ERROR("Failed to open file: ", fileName);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
	}

    VkShaderModule GraphicsPipeline::CreateShaderModule(const std::vector<char>& shaderCode) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(Engine::s_PhysicalDevice->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
}