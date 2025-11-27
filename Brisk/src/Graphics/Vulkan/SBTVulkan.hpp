#pragma once

#include "Engine/Renderer/SBT.hpp"

#include <volk.h>

namespace Brisk
{
	class SBTVulkan : public SBT {
	public:
		virtual void Init(Type type, uint32_t count) override;
	private:
		VkStridedDeviceAddressRegionKHR region;
	};
}