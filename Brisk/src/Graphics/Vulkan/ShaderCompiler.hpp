#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <volk.h>
#include <glslang/Public/ShaderLang.h>

namespace Brisk 
{
    bool CompileShaderToSPIRV(const std::string& sourceCode, const std::string& shaderName, VkShaderStageFlagBits stage, std::vector<uint32_t>& spirv) {
        // Initialize glslang
        glslang::InitializeProcess();

        // Create an instance of the glslang shader object
        //glslang::TShader shader(static_cast<glslang::EShLanguage>(stage));

        // Set the shader source code
        const char* sourceCStr = sourceCode.c_str();
        //shader.setStrings(&sourceCStr, 1);

        // Set the entry point name
        //shader.setEntryPoint("main");

        // Compile the shader
        EShMessages messages = EShMsgDefault;
        bool compiled;
        //compiled = shader.parse(nullptr, 100, false, messages);

        if (!compiled) {
            //std::cerr << "Shader compilation failed: " << shader.getInfoLog() << std::endl;
            glslang::FinalizeProcess();
            return false;
        }

        // Link the shader into SPIR-V
        glslang::TProgram program;
        //program.addShader(&shader);

        bool linked = program.link(messages);
        if (!linked) {
            std::cerr << "Shader linking failed: " << program.getInfoLog() << std::endl;
            glslang::FinalizeProcess();
            return false;
        }

        // Convert the linked program to SPIR-V binary format
        //glslang::SpvOptions options;
        //program.convertToSpv(spirv, &options);

        glslang::FinalizeProcess();
        return true;
    }

    void WriteSPIRVToFile(const std::string& filename, const std::vector<uint32_t>& spirv) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }
        file.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
        file.close();
        std::cout << "SPIR-V written to: " << filename << std::endl;
    }

    // Example Usage
    //std::vector<uint32_t> spirv;

    //// Compile the shader to SPIR-V
    //if (CompileShaderToSPIRV(shaderCode, "simple_fragment_shader", VK_SHADER_STAGE_FRAGMENT_BIT, spirv)) {
    //    // Write the SPIR-V binary to a file
    //    WriteSPIRVToFile("simple_fragment_shader.spv", spirv);
    //}
}