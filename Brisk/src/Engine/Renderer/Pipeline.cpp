// INCLUDES
#include "pch.hpp"
#include "Pipeline.hpp"
#include "Graphics/Vulkan/PipelineVulkan.hpp"
#include "Engine/Engine.hpp"

#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/PipelineDirectX12.hpp"
#endif
//------------------------------------------

namespace Brisk 
{
    std::shared_ptr<Pipeline> Pipeline::Create() {
        if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
            return std::make_shared<PipelineVulkan>();
        }
#ifdef BRISK_ENABLE_DIRECTX12
        else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
            return std::make_shared<PipelineDirectX12>();
        }
#endif
    }
}