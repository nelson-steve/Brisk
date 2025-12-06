#pragma once

#include "Engine/Renderer/SBT.hpp"

#include <volk.h>

namespace Brisk
{
	class SBTVulkan : public SBT {
	public:
		virtual void Init(std::shared_ptr<Pipeline> pipeline) override;
		const VkStridedDeviceAddressRegionKHR* GetRaygenRegion() { return &rayGen; }
		const VkStridedDeviceAddressRegionKHR* GetMissRegion() { return &miss; }
		const VkStridedDeviceAddressRegionKHR* GetHitRegion() { return &hit; }
		const VkStridedDeviceAddressRegionKHR* GetCallableRegion() { return &callable; }
	private:
		VkStridedDeviceAddressRegionKHR rayGen;
		VkStridedDeviceAddressRegionKHR miss;
		VkStridedDeviceAddressRegionKHR hit;
		VkStridedDeviceAddressRegionKHR callable;
	};
}