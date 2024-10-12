#include "Pipeline.hpp"
#include "Graphics/Vulkan/PipelineVulkan.hpp"

namespace Brisk {
    std::shared_ptr<Pipeline> Pipeline::Create(const PipelineSpecs& specs) {
        return std::make_shared<PipelineVulkan>(specs);
    }
}