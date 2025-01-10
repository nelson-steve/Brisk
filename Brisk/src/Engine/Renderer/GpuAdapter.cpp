// INCLUDES
#include "GpuAdapter.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"
//--------------------------------------------

namespace Brisk
{
	std::shared_ptr<GpuAdapter> GpuAdapter::Create() {
		return std::make_shared<GpuAdapterVulkan>();
	}
}