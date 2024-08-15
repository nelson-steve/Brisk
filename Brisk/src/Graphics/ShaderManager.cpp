#include "ShaderManager.hpp"

namespace Brisk {
    std::vector<char>* ShaderManager::m_ShaderFileBuffer;

    std::vector<char>& ShaderManager::ReadShaderFile(const std::string& fileName) {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            BRISK_CORE_ERROR("Failed to open file: ", fileName);
        }

        size_t fileSize = (size_t)file.tellg();
        m_ShaderFileBuffer = new std::vector<char>(fileSize);

        file.seekg(0);
        file.read(m_ShaderFileBuffer->data(), fileSize);

        file.close();

        return m_ShaderFileBuffer[0];
    }

    const ShaderModule& ShaderManager::CreateShaderModule(const ShaderInfo& info) {
        const std::vector<char>& shaderCode = ReadShaderFile(info.Path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        ShaderModule module;
        if (vkCreateShaderModule(Engine::s_PhysicalDevice->GetDevice(), &createInfo, nullptr, &module.Module) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        // Cleanup the data that's not needed anymore
        delete m_ShaderFileBuffer;

        module.Type = info.Type;

        return module;
    }

    const void ShaderManager::ReleaseShaderModule(const std::vector<ShaderModule>& modules) {
        for(auto& module : modules)
            vkDestroyShaderModule(Engine::s_PhysicalDevice->GetDevice(), module.Module, nullptr);
    }
}