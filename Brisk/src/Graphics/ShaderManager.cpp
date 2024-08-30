#include "ShaderManager.hpp"
#include "Graphics/Vulkan/GpuContextVulkan.hpp"

namespace Brisk {

    std::vector<char>* ShaderManager::ReadShaderFile(const std::string& fileName) {
        std::vector<char>* shaderFileBuffer;
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            BRISK_CORE_ERROR("Failed to open file: ", fileName);
        }

        size_t fileSize = (size_t)file.tellg();
        shaderFileBuffer = new std::vector<char>(fileSize);

        file.seekg(0);
        file.read(shaderFileBuffer->data(), fileSize);

        file.close();

        return shaderFileBuffer;
    }

    const VkShaderModule ShaderManager::CreateShaderModule(const std::string& path) {
        const std::vector<char>* shaderCode = ReadShaderFile(path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode->size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());

        VkShaderModule  module;
        if (vkCreateShaderModule(GpuContextVulkan::s_GpuDevice->GetDevice(), &createInfo, nullptr, &module) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        // Cleanup the data that's not needed anymore
        delete shaderCode;
        return module;
    }
}