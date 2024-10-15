#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace Brisk
{
	class Descriptor {
	public:
		enum DescriptorType {
			UNIFORM,
			IMAGE,
		};

		struct Layout {
			uint32_t p_Binding;
			uint32_t p_DescriptorCount;
			DescriptorType p_Type;
		};

	public:
		virtual void Init() = 0;
		virtual void Allocate() = 0;
		virtual void Update() = 0;

		void AddBindingLayout(uint32_t binding, uint32_t count, Descriptor::DescriptorType type) {
			Layout layout{};
			layout.p_Binding = binding;
			layout.p_DescriptorCount = count;
			layout.p_Type = type;
			m_Layouts.push_back(layout);
		}

		static std::shared_ptr<Descriptor> Create();
	protected:
		std::vector<Layout> m_Layouts;
	};
}