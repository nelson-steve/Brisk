#pragma once

#include "Graphics/Vulkan/BufferVulkan.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace Brisk
{
	class DescriptorLayout {
	public:
		enum DescriptorType
		{
			SAMPLER,
			COMBINED_IMAGE_SAMPLER,
			SAMPLED_IMAGE,
			STORAGE_IMAGE,
			UNIFORM_BUFFER,
			STORAGE_BUFFER,
		};

		struct Layout {
			uint32_t p_Binding;
			uint32_t p_DescriptorCount;
			DescriptorType p_Type;
		};
		void AddBindingLayout(uint32_t binding, uint32_t count, DescriptorType type) {
			Layout layout{};
			layout.p_Binding = binding;
			layout.p_DescriptorCount = count;
			layout.p_Type = type;
			m_Layouts.push_back(layout);
		}
		void AddBindingLayout(const std::vector<Layout> layouts) {
			for (const Layout& l : layouts) {
				Layout layout{};
				layout.p_Binding = l.p_Binding;
				layout.p_DescriptorCount = l.p_DescriptorCount;
				layout.p_Type = l.p_Type;
				m_Layouts.push_back(l);
			}
		}
	public:
		virtual void Init() = 0;

		static std::shared_ptr<DescriptorLayout> Create();
	protected:
		std::vector<Layout> m_Layouts;
	};

	class Descriptor {
	public:
		virtual void Allocate() = 0;
		virtual void Update(BufferVulkan* buffer) = 0;

		static std::shared_ptr<Descriptor> Create();
	protected:
		std::shared_ptr<DescriptorLayout> m_Layout;
	};
}