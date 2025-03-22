// INCLUDES
#include "ShaderModule.hpp"
#include "Graphics/Vulkan/ShaderModuleVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/DirectX12/ShaderModuleDirecx12.hpp"
//-----------------------------------------------

namespace Brisk 
{
    std::shared_ptr<ShaderModule> ShaderModule::Create() 
    { 
		if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
			return std::make_shared<ShaderModuleVulkan>();
		}
		else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
			return std::make_shared<ShaderModuleDirectX12>();
		}
    }
}