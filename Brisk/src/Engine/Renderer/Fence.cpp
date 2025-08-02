// INCLUDES
#include "Fence.hpp"
#include "Graphics/Vulkan/FenceVulkan.hpp"
#include "Engine/Engine.hpp"
#ifdef BRISK_ENABLE_DIRECTX12
#include "Graphics/DirectX12/FenceDirectX12.hpp"
#endif
//----------------------------------------

namespace Brisk 
{
    std::shared_ptr<Fence> Fence::Create(){
        if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::Vulkan) {
            return std::make_shared<FenceVulkan>();
        }
#ifdef BRISK_ENABLE_DIRECTX12
        else if (Engine::s_EngineSettings.API == Engine::EngineSettings::GraphicsAPI::DirectX12) {
            return std::make_shared<FenceDirectX12>();
        }
#endif
    }
}