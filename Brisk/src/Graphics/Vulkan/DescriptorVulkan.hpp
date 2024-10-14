#pragma once 

#include "Engine/Renderer/Descriptor.hpp"

#include <Volk/volk.h>

namespace Brisk
{
	class DescriptorVulkan : public Descriptor {
	public:
		DescriptorVulkan();
		virtual void Allocate() override;
		virtual void Update() override;
	private:
		VkDescriptorSet m_Set;
		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts;
	};
}