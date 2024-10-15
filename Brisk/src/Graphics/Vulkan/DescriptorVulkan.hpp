#pragma once 

#include "Engine/Renderer/Descriptor.hpp"

#include <Volk/volk.h>

namespace Brisk
{
	class DescriptorVulkan : public Descriptor {
	public:
		virtual void Init() override;
		virtual void Allocate() override;
		virtual void Update() override;

		void SetPool(VkDescriptorPool pool) { m_Pool = pool; }

		const std::vector<VkDescriptorSetLayout> GetLayouts() const { return m_DescriptorLayouts; }
	private:
		VkDescriptorSet m_Set;
		VkDescriptorPool m_Pool;
		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts;
	};
}