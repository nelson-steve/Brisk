#include "Shader.hpp"
#include "Graphics/Vulkan/ShaderVulkan.hpp"

namespace Brisk
{
	std::shared_ptr<Shader> Shader::Create() {
		return std::make_shared<ShaderVulkan>();
	}
}