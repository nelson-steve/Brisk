#pragma once

#include <cstdint>
#include <memory>

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
		virtual void AddLayout() = 0;
		virtual void Allocate() = 0;
		virtual void Update() = 0;

		static std::shared_ptr<Descriptor> Create();
	};
}